#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include <stdio.h>
#include "LCD.hpp"

// Sensor de humedad
static constexpr adc_unit_t    MoistureUnit = ADC_UNIT_1;
static constexpr adc_channel_t MoistureCh   = ADC_CHANNEL_6; // GPIO34

static adc_oneshot_unit_handle_t adc_handle = nullptr;

// Relé de la bomba
static constexpr gpio_num_t PumpPin = GPIO_NUM_2;

// Botones
static constexpr gpio_num_t ModePin = GPIO_NUM_15; 
static constexpr gpio_num_t PeriodPin = GPIO_NUM_4;
static constexpr gpio_num_t IrrTimePin = GPIO_NUM_5;
static constexpr gpio_num_t MinMoisPin = GPIO_NUM_18;

// Pines para LCD en ESP32 (4-bit mode)
static constexpr gpio_num_t LCD_RS = GPIO_NUM_13;
static constexpr gpio_num_t LCD_EN = GPIO_NUM_12;
static constexpr gpio_num_t LCD_D4 = GPIO_NUM_14;
static constexpr gpio_num_t LCD_D5 = GPIO_NUM_27;
static constexpr gpio_num_t LCD_D6 = GPIO_NUM_26;
static constexpr gpio_num_t LCD_D7 = GPIO_NUM_25;

// === Variables globales compartidas ===
static volatile int Mode = 2;
static volatile int IrrigationPeriod = 15;  // s
static volatile int IrrigationTime = 3;   // s
static volatile int MoistureThresh = 30;  // %
static volatile int currMoisture = 0;


// === Prototipos de tareas ===
extern "C" void sensor_task(void*);
extern "C" void switch_task(void*);
extern "C" void control_task(void*);
extern "C" void lcd_task(void*);

// === Prototipos de funciones ===
extern "C" void init_uart();

// === SET UP ===
// === Punto de entrada de ESP-IDF ===
extern "C" void app_main(){
    // --- Inicializar UART0 para stdout/stderr (para debbuging) ---
    //init_uart();

    // --- Inicializar GPIO y ADC ---
    gpio_set_direction(PumpPin, GPIO_MODE_OUTPUT);
    gpio_set_level(PumpPin, 0);

    auto init_button = [](gpio_num_t pin){
        gpio_set_direction(pin, GPIO_MODE_INPUT);
        gpio_pullup_en(pin);
    };
    init_button(ModePin);
    init_button(PeriodPin);
    init_button(IrrTimePin);
    init_button(MinMoisPin);

    // --- Inicializar ADC ---
    adc_oneshot_unit_init_cfg_t init_cfg = {
    MoistureUnit,                   // unit_id
    (adc_oneshot_clk_src_t)0,       // clk_src = 0 → default clock source
    ADC_ULP_MODE_DISABLE            // ulp_mode
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    // atten, bitwidth
    adc_oneshot_chan_cfg_t chan_cfg = {
        ADC_ATTEN_DB_12,                // atten
        ADC_BITWIDTH_DEFAULT            // bitwidth
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, MoistureCh, &chan_cfg));

    // --- Inicializar LCD ---
    lcdInit(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
    // --- Crear las tareas ---
    xTaskCreate(sensor_task,  "Sensor",  2048, nullptr, 7, nullptr);
    xTaskCreate(switch_task,  "Switch",  2048, nullptr, 6, nullptr);
    xTaskCreate(control_task, "Control", 2048, nullptr, 5, nullptr);
    xTaskCreate(lcd_task,     "LCD",     2048, nullptr, 4, nullptr);
}

void init_uart(){
    const uart_port_t uart_num = UART_NUM_0;
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_driver_install(uart_num, 256, 0, 0, nullptr, 0));
    // Vincula stdout, stdin y stderr a UART0
    esp_vfs_dev_uart_use_driver(uart_num);

    // (opcional) Desactivar buffering de stdout para ver cada printf inmediatamente:
    setvbuf(stdout, nullptr, _IONBF, 0);

    // Ahora sí tus printf/ESP_LOGI() aparecerán en idf.py monitor
    printf("UART inicializado a 115200 bps\n");
}

// === Implementación de tareas ===
void sensor_task(void*){
    for (;;)
    {
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, MoistureCh, &raw));
        currMoisture = raw * 100 / 4095;
        //printf("Moisture: %d%%\n", currMoisture); // Para debbugging 
        // Delay para ahorro de energía
        vTaskDelay(pdMS_TO_TICKS(4000)); //700
    }
}

void switch_task(void*){
    for (;;)
    {
        if (gpio_get_level(ModePin) == 0) {
            Mode = (Mode == 1) ? 2 : 1;
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        if (gpio_get_level(PeriodPin) == 0) {
            IrrigationPeriod = (IrrigationPeriod < 20) ? IrrigationPeriod + 5 : 10;
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        if (gpio_get_level(IrrTimePin) == 0) {
            IrrigationTime = (IrrigationTime < 5) ? IrrigationTime + 2 : 1;
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        if (gpio_get_level(MinMoisPin) == 0) {
            MoistureThresh = (MoistureThresh < 45) ? MoistureThresh + 15 : 15;
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        // Delay para ahorro de energía
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void control_task(void*){
    int timer = 0;
    bool eventFlag = false;

    for (;;)
    {
        timer++;
        switch (Mode) {
        case 1:
            gpio_set_level(PumpPin, (currMoisture < MoistureThresh));
            break;
        case 2:
            if (!eventFlag && timer >= IrrigationPeriod) {
                eventFlag = true;
                timer = 0;
            }
            if (eventFlag && timer < IrrigationTime) {
                gpio_set_level(PumpPin, 1);
            } else {
                gpio_set_level(PumpPin, 0);
                if (eventFlag && timer >= IrrigationTime) {
                    eventFlag = false;
                    timer = 0;
                }
            }
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        //printf("Timer=%ds\n", timer); // Para debbugging 
    }
}

void lcd_task(void*){
    for (;;)
    {
        lcdClear();
        // Línea 1
        lcdSetCursor(0, 0);
        char buf1[17];
        snprintf(buf1, sizeof(buf1), "M:%d P:%ds HL:%ds",
                 Mode, IrrigationPeriod, IrrigationTime);
        lcdPrint(buf1);

        // Línea 2
        lcdSetCursor(0, 1);
        char buf2[17];
        snprintf(buf2, sizeof(buf2), "mM:%d%% C:%d%%",
                 MoistureThresh, currMoisture);
        lcdPrint(buf2);

        // Delay para ahorro de energía
        vTaskDelay(pdMS_TO_TICKS(3000));
        //printf("LCD actualizado\n"); // Para debbugging 
    }
}

