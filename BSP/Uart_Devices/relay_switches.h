#ifndef RELAY_SWITCHES_H
#define RELAY_SWITCHES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"

#define RELAY_UART huart1

typedef enum {
    RELAY_INDEX_SUCK = 1,
    RELAY_INDEX_ZHUAZHUA = 2
} relay_index_t;

void relay_open(relay_index_t index);
void relay_close(relay_index_t index);

#ifdef __cplusplus
}
#endif

#endif