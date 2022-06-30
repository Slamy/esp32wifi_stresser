
#ifdef __cplusplus
extern "C"
{
#endif

	void wifi_init_sta();

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

	extern EventGroupHandle_t s_wifi_event_group;

#ifdef __cplusplus
}
#endif
