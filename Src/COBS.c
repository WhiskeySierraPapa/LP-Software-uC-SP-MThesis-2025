/*
 * COBS.c
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */


#include "COBS.h"

/** COBS encode data to buffer
	@param data Pointer to input data to encode
	@param length Number of bytes to encode
	@param buffer Pointer to encoded output buffer
	@return Encoded buffer length in bytes
	@note Does not output delimiter byte
*/
size_t COBS_encode(const void *data, size_t length, uint8_t *buffer) {
	assert(data && buffer);

	uint8_t *p_output = buffer + 1; // Pointer to first byte of encoded output.
	uint8_t *p_zero_byte = buffer; // Pointer to the overhead byte. Later used to point to zero bytes that are replaced.
	uint8_t code = 1; // Value that is used to replace the zero bytes. Denotes after how many bytes next zero byte is located.

	for (const uint8_t *p_input_byte = (const uint8_t *)data; length--; p_input_byte++) {
		if (*p_input_byte) {// Byte not zero, write it
			*p_output++ = *p_input_byte;
			code++;
		}

		if (!*p_input_byte || code == 0xff) {// Input is zero or block completed, restart
			*p_zero_byte = code;
			code = 1;
			p_zero_byte = p_output;
			if (!*p_input_byte || length) {
				p_output++;
			}
		}
	}
	*p_zero_byte = code; // Write final code value

	return (size_t)(p_output - buffer);
}


/** COBS decode data from buffer
	@param buffer Pointer to encoded input bytes
	@param length Number of bytes to decode
	@param data Pointer to decoded output data
	@return Number of bytes successfully decoded
	@note Stops decoding if delimiter byte is found
*/
void COBS_decode(const uint8_t *buffer, size_t length, void *data) {

	if (length < 2 || buffer[length - 1] != 0x00) {
	        return 0;
	}

	const uint8_t *p_input_byte = buffer; // Pointer to input byte
	uint8_t *p_output = (uint8_t *)data; // Pointer to output buffer

	uint8_t dist_next_zero = 0;
	uint8_t code = 0xff;
	while (p_input_byte < buffer + length) {
		if (dist_next_zero) { // Have not reached a zero byte yet, thus just copy.
			*p_output++ = *p_input_byte++;
		} else {
			dist_next_zero = *p_input_byte++; // Reached an encoded zero byte, save distance to next.

			if (dist_next_zero && (code != 0xff)) { // Decode zero byte.
				*p_output++ = 0;
			}

			code = dist_next_zero;

			if (!code) {// Delimiter code (0x00) found
				break;
			}
		}
		dist_next_zero--;
	}
}


/** COBS verify if frame is valid
	@param buffer Pointer to encoded input bytes
	@param length Number of bytes to verify
*/
uint8_t COBS_is_valid(const uint8_t *buffer, size_t length)
{
	if (length < 2 || buffer[length - 1] != 0x00) {
		return 0; // Must end with 0x00
	}

	size_t index = 0;

	while (index < length - 1) {
		uint8_t code = buffer[index]; // Get the code byte

		if (code == 0 || index + code > length) {
			return 0; // Code byte cannot be zero or exceed buffer length
		}

		index += code; // Move to the next code byte

		// If we haven't reached the end, the next byte must be another code
		if (index < length - 1 && buffer[index] == 0) {
			return 0; // Unexpected zero found
		}
	}

	return 1;
}

