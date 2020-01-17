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
#define LEN_USER_ID             4
#define LEN_ACTION              1
#define LEN_TOPIC               15
#define LEN_WEIGHT              3
#define MAPPING_COUNT           5

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                            EXTERNS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

// The RULE_ACTIONS and RULE_WEIGHTS below define how each action is mapped
// to the appropriate weight.
extern const char RULE_ACTION[MAPPING_COUNT];
extern const int32_t RULE_WEIGHT[MAPPING_COUNT];

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int32_t console_string_read(char* store_string_location);
void console_string_write(char* string, uint32_t len);

#endif