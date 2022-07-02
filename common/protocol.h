/*
 * protocol.h
 *
 *  Created on: 30.06.2022
 *      Author: andre
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <cstdint>

typedef struct
{
	uint32_t frame_counter;
	int8_t max_tx_power;
} __attribute__((packed)) PackagePcToEsp;

typedef struct
{
	uint32_t frame_counter;
	uint32_t lost_frames;
	uint32_t rssi;
	uint32_t err_wrong_order_cnt;
	uint32_t err_sendto_cnt;
	uint32_t err_recvfrom_cnt;
	uint32_t info_reset_cnt;
	int8_t current_max_tx_power;
} __attribute__((packed)) PackageEspToPc;

#endif /* PROTOCOL_H_ */
