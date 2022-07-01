#include "asio.hpp"
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
#include <optional>

void handle_send(std::error_code ec, std::size_t bytes_sent);
void handle_receive(std::error_code ec, std::size_t bytes_recvd);

static const char* TAG = "espstresser";

using asio::ip::udp;

static uint32_t total_lost_frames		   = 0;
static int32_t last_received_frame_counter = -1;
std::optional<asio::io_context> io_context;
std::optional<udp::socket> sock;
udp::endpoint sender_endpoint;

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

extern "C" void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	wifi_init_sta();

	io_context.emplace();
	sock.emplace(*io_context, udp::endpoint(udp::v4(), 10000));
	sock->async_receive_from(asio::buffer(recv_buffer.raw.data(), recv_buffer.raw.size()), sender_endpoint,
							 handle_receive);
	io_context->run();
}

void handle_receive(std::error_code ec, std::size_t bytes_recvd)
{
	if (!ec && bytes_recvd > 0)
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

		ESP_LOGI(TAG, "Received %d %d %d", bytes_recvd, recv_buffer.content.frame_counter, total_lost_frames);

		last_received_frame_counter = recv_buffer.content.frame_counter;

		wifi_ap_record_t ap_info{};
		esp_wifi_sta_get_ap_info(&ap_info);

		send_buffer.content.frame_counter++;
		send_buffer.content.rssi		= ap_info.rssi;
		send_buffer.content.lost_frames = total_lost_frames;

		sock->async_send_to(asio::buffer(send_buffer.raw.data(), send_buffer.raw.size()), sender_endpoint, handle_send);
	}
	else
	{
		ESP_LOGE(TAG, "Error occurred during reception: ec %d", ec);
		send_buffer.content.err_recvfrom_cnt++;
	}

	sock->async_receive_from(asio::buffer(recv_buffer.raw.data(), recv_buffer.raw.size()), sender_endpoint,
							 handle_receive);
}

void handle_send(std::error_code ec, std::size_t bytes_sent)
{
	if (ec)
	{
		ESP_LOGE(TAG, "Error occurred during sending: ec %d", ec);
		send_buffer.content.err_sendto_cnt++;
	}
}
