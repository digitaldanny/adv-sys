#ifndef _SRC_COMMON_HEADER_
#define _SRC_COMMON_HEADER_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                            DEFINES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

#define ENTER                   10
#define LB                      '('
#define RB                      ')'
#define DELIMITER               ','
#define SPACE                   32
#define MAX_INPUT_STRING_LENGTH 50

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int32_t console_string_read(char* store_string_location);
void console_string_write(char* string, uint32_t len);

#endif