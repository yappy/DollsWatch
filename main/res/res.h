/*
 * See
 * https://docs.espressif.com/projects/esp-idf/en/stable/api-guides/build-system.html
 * Embedding Binary Data
 */

#ifndef RES_RES_H_
#define RES_RES_H_

extern const uint8_t file_server_html_start[]
	asm("_binary_file_server_html_start");
extern const uint8_t file_server_html_end[]
	asm("_binary_file_server_html_end");

#endif // RES_RES_H_
