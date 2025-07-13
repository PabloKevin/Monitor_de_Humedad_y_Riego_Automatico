#pragma once
#include "driver/gpio.h"
#include <stdint.h>

/**
 * @brief Inicializa el LCD en modo 4-bits usando los pines indicados.
 * @param rs   Pin RS
 * @param en   Pin EN
 * @param d4   Pin D4
 * @param d5   Pin D5
 * @param d6   Pin D6
 * @param d7   Pin D7
 */
void lcdInit(gpio_num_t rs,
             gpio_num_t en,
             gpio_num_t d4,
             gpio_num_t d5,
             gpio_num_t d6,
             gpio_num_t d7);

void lcdPulseEnable();
void lcdWrite4(uint8_t nibble);
void lcdCommand(uint8_t cmd);
void lcdWriteChar(char c);

// Limpia la pantalla
void lcdClear();
// Sitúa el cursor
void lcdSetCursor(uint8_t col, uint8_t row);
// Escribe una cadena
void lcdPrint(const char* str);