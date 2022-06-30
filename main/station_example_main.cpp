/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_event.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include <string.h>
#include <sys/param.h>

#include "../pctool/protocol.h"
#include "wifi.h"

#include <array>

static const char* TAG = "espstresser";

extern "C" void app_main(void);

union
{
	PackageEspToPc content;
	std::array<uint8_t, 1000> raw;
} send_buffer;
union
{
	PackagePcToEsp content;
	std::array<uint8_t, 1000> raw;
} recv_buffer;

static uint32_t total_lost_frames		   = 0;
static int32_t last_received_frame_counter = -1;

void app_main(void)
{
	struct sockaddr_in6 dest_addr;
	int ip_protocol = 0;

	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	wifi_init_sta();

	struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in*)&dest_addr;
	dest_addr_ip4->sin_addr.s_addr	  = htonl(INADDR_ANY);
	dest_addr_ip4->sin_family		  = AF_INET;
	dest_addr_ip4->sin_port			  = htons(10000);
	ip_protocol						  = IPPROTO_IP;

	int sock = socket(AF_INET, SOCK_DGRAM, ip_protocol);
	if (sock < 0)
	{
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		return;
	}
	ESP_LOGI(TAG, "Socket created");

	int err = bind(sock, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
	if (err < 0)
	{
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
	}
	ESP_LOGI(TAG, "Socket bound, port %d", 10000);

	struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
	socklen_t socklen = sizeof(source_addr);

	while (1)
	{
		int len =
			recvfrom(sock, recv_buffer.raw.data(), recv_buffer.raw.size(), 0, (struct sockaddr*)&source_addr, &socklen);

		if (len > 0)
		{
			if (last_received_frame_counter > 0)
			{
				int32_t lost_frames = recv_buffer.content.frame_counter - (last_received_frame_counter + 1);

				if (lost_frames < -70)
				{
					last_received_frame_counter = -1;
					total_lost_frames			= 0;
					send_buffer.content.info_reset_cnt++;
					ESP_LOGI(TAG, "Reset");
				}
				else if (lost_frames >= 0)
				{
					total_lost_frames += lost_frames;
				}
				else
				{
					send_buffer.content.err_wrong_order_cnt++;
				}
			}

			ESP_LOGI(TAG, "Received %d %d %d", len, recv_buffer.content.frame_counter, total_lost_frames);

			last_received_frame_counter = recv_buffer.content.frame_counter;

			wifi_ap_record_t ap_info{};
			esp_wifi_sta_get_ap_info(&ap_info);

			send_buffer.content.frame_counter++;
			send_buffer.content.rssi		= ap_info.rssi;
			send_buffer.content.lost_frames = total_lost_frames;

			int err = sendto(sock, send_buffer.raw.data(), send_buffer.raw.size(), 0, (struct sockaddr*)&source_addr,
							 sizeof(source_addr));
			if (err < 0)
			{
				ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
				send_buffer.content.err_sendto_cnt++;
			}
		}
		else
		{
			send_buffer.content.err_recvfrom_cnt++;
		}
	}
}
