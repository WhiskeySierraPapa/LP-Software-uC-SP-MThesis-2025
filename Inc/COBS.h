/*
 * COBS.h
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs
 */

#ifndef COBS_H_
#define COBS_H_

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

size_t cobsEncode(const void *data, size_t length, uint8_t *buffer);

size_t cobsDecode(const uint8_t *buffer, size_t length, void *data);


#endif /* COBS_H_ */
