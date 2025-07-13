#include "LCD.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// === Funciones de manejo del LCD ===

// Variables internas que guardan los pines actuales
static gpio_num_t LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7;

void lcdInit(gpio_num_t rs,
             gpio_num_t en,
             gpio_num_t d4,
             gpio_num_t d5,
             gpio_num_t d6,
             gpio_num_t d7) {
    
    // Guarda los pines
    LCD_RS = rs; LCD_EN = en;
    LCD_D4 = d4; LCD_D5 = d5;
    LCD_D6 = d6; LCD_D7 = d7;
    
    // Configurar pines como salidas
    gpio_set_direction(LCD_RS, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_EN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D4, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D5, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D6, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D7, GPIO_MODE_OUTPUT);

    vTaskDelay(pdMS_TO_TICKS(50));  // Espera power-on

    // Secuencia de arranque 4-bit
    gpio_set_level(LCD_RS, 0);
    for (int i = 0; i < 3; ++i) {
        lcdWrite4(0x03);
        vTaskDelay(pdMS_TO_TICKS(i ? 1 : 5));
    }
    lcdWrite4(0x02);  // Ya en 4-bit

    // Función: 4-bit, 2 líneas, fuente 5×8
    lcdCommand(0x28);
    // Display ON, cursor OFF, blink OFF
    lcdCommand(0x0C);
    // Clear display
    lcdCommand(0x01);
    vTaskDelay(pdMS_TO_TICKS(2));
    // Entry mode: incrementa cursor, sin shift
    lcdCommand(0x06);
}

void lcdPulseEnable() {
    gpio_set_level(LCD_EN, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(LCD_EN, 1);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(LCD_EN, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
}

void lcdWrite4(uint8_t nibble) {
    gpio_set_level(LCD_D4, (nibble >> 0) & 1);
    gpio_set_level(LCD_D5, (nibble >> 1) & 1);
    gpio_set_level(LCD_D6, (nibble >> 2) & 1);
    gpio_set_level(LCD_D7, (nibble >> 3) & 1);
    lcdPulseEnable();
}

void lcdCommand(uint8_t cmd) {
    gpio_set_level(LCD_RS, 0);
    lcdWrite4(cmd >> 4);
    lcdWrite4(cmd & 0x0F);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcdWriteChar(char c) {
    gpio_set_level(LCD_RS, 1);
    lcdWrite4(c >> 4);
    lcdWrite4(c & 0x0F);
    vTaskDelay(pdMS_TO_TICKS(1));
}

void lcdClear() {
    lcdCommand(0x01);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcdSetCursor(uint8_t col, uint8_t row) {
    static const uint8_t rowOffsets[] = { 0x00, 0x40, 0x14, 0x54 };
    lcdCommand(0x80 | (col + rowOffsets[row]));
}

void lcdPrint(const char *str) {
    while (*str) {
        lcdWriteChar(*str++);
    }
}