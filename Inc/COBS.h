/*
 * COBS.h
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#ifndef COBS_H_
#define COBS_H_

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#define COBS_FRAME_LEN 	256

size_t COBS_encode(const void *data, size_t length, uint8_t *buffer);

void COBS_decode(const uint8_t *buffer, size_t length, void *data);

uint8_t COBS_is_valid(const uint8_t *buffer, size_t length);


#endif /* COBS_H_ */
