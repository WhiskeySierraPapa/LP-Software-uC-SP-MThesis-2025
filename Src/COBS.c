/*
 * COBS.c
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs
 */


#include "COBS.h"

/** COBS encode data to buffer
	@param data Pointer to input data to encode
	@param length Number of bytes to encode
	@param buffer Pointer to encoded output buffer
	@return Encoded buffer length in bytes
	@note Does not output delimiter byte
*/
size_t cobsEncode(const void *data, size_t length, uint8_t *buffer) {
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
size_t cobsDecode(const uint8_t *buffer, size_t length, void *data) {
	assert(buffer && data);

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

	return (size_t)(p_output - (uint8_t *)data);
}

/*
int main() {
	#define INPUT_LEN 512
	#define OUTPUT_LEN 514
	uint8_t input[INPUT_LEN];
	uint8_t output[OUTPUT_LEN];
	uint8_t decoded[INPUT_LEN];
	for(int i = 0; i < INPUT_LEN; i++) {
		//if (i % 5 == 0) {
		//	input[i] = 0;
		//} else {
		input[i] = 244;
		//}
		printf("%d ", input[i]);
	}
	printf("\n");
	printf("\n");

	cobsEncode(input, INPUT_LEN, output);
	cobsDecode(output, INPUT_LEN, decoded);

	for(int i = 0; i < OUTPUT_LEN; i++) {
		printf("%d ", output[i]);
	}
	printf("\n");printf("\n");

	for(int i = 0; i < INPUT_LEN; i++) {
		printf("%d ", decoded[i]);
	}
	printf("\n");

	for(int i = 0; i < INPUT_LEN; i++) {
		if (input[i] != decoded[i])
			printf("DECODE DOES NOT MATCH! %d", i);
	}


}
*/
