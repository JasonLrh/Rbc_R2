#include "relay_switches.h"

static inline void trans(uint8_t code){
    static uint8_t __section(".dma_data") mao;
    static uint8_t __bck = 0x00000000;
    mao = code | (code << 6);
    if (__bck != mao){
        HAL_UART_Transmit(&RELAY_UART, &mao, 1, HAL_MAX_DELAY);
        __bck = mao;
    }
}

static uint8_t current_state = 0;

void relay_open(relay_index_t index){
    current_state |= index;
    trans(current_state);
}

void relay_close(relay_index_t index){
    current_state &= ~(index);
    trans(current_state);
}