#         MONITOR DE HUMEDAD Y RIEGO AUTOMÁTICO
#                LABORATORIO III – SISTEMAS EMBEBIDOS

Autores:
- Pablo Kevin Pereira - pablo.pereira.p@estudiantes.utec.edu.uy
- Tomás Eduardo Conti - tomas.conti@estudiantes.utec.edu.uy

Universidad Tecnológica del Uruguay (UTEC) – Ingeniería Mecatrónica

## Resumen

Se presenta la implementación modular de un sistema embebido de riego automatizado, desarrollado íntegramente sobre plataforma ESP32 utilizando FreeRTOS. La programación fue realizada tomando como referencia una implementación física previa, respetando su estructura lógica, pines y comportamiento.

El sistema se diseñó dividiendo las funcionalidades en tareas concurrentes: adquisición de humedad, control de bomba, actualización de LCD y lectura de botones. Esto permite validar el modelo de ejecución multitarea en condiciones realistas y demostrar que el firmware es robusto y listo para desplegar.

## Herramientas


- Microcontrolador: ESP32-WROOM-32
- Sistema operativo: GNU/Linux Ubuntu
- Entorno de desarrollo: Visual Studio Code + ESP-IDF
- Framework de desarrollo: FreeRTOS (ESP-IDF)

## Estructura del repositorio
    Lab1_Sistemas_Embebidos/  → Implementación enfocada en circuito electrónico y SysML
    Lab2_Sistemas_Embebidos/  → Medición de tiempos reales de tareas
    Lab3_RTOS/                → Implementación de tareas con RTOS * Resultado Final *
    ├── .vscode/              → Configuraciones del entorno
    ├── build/                → Archivos generados
    ├── CMakeLists.txt        → Script principal del proyecto
    ├── sdkconfig             → Configuración global del sistema
    └── main/                 → Firmware principal y librerías
        ├── CMakeLists.txt
        ├── Lab3_main.cpp     → app_main(), tareas FreeRTOS
        ├── LCD.hpp           → Declaraciones de interfaz LCD
        └── LCD.cpp           → Implementación de LCD

## Tareas FreeRTOS implementadas

sensor_task (prioridad 7)     → Lee humedad cada determinado tiempo y guarda en registro
switch_task (prioridad 6)     → Detecta pulsaciones de botones y ejecuta acciones correspondientes
control_task (prioridad 5)    → Aplica lógica de riego con modos y tiemops
lcd_task (prioridad 4)        → Refresca la LCD

Todas se ejecutan sobre el core 0 (PRO CPU). El core 1 queda libre.

## Identificación de puerto USB y carga del firmware

- Puerto detectado como: /dev/ttyUSB0
- Visualizado vía comandos: `lsusb` y `ls /dev/ttyUSB*`
- Uso de `idf.py flash` y `idf.py monitor` para carga y debugging

## Observaciones técnicas

✔️ El sistema funcionó correctamente sin sensores reales: se usó contacto humano sobre el pin analógico para probar el ADC.

✔️ El GPIO que controla la bomba se asignó al LED azul on-board, lo cual permite observar visualmente la activación del sistema.

⚠️ Se detectó un reinicio constante debido a tareas sin vTaskDelay(). Esto generaba bloqueo del watchdog y reseteo de la ESP32. Solución: todas las tareas incluyen retardos mínimos.

💡 Nota importante: FreeRTOS usa “ticks” como unidad temporal. Cada tick se genera por interrupción (ej.: 1ms si configTICK_RATE_HZ = 1000). Todos los delays son múltiplos de esa unidad.

## Resultados obtenidos

- Lecturas de humedad desde el ADC mostradas por consola

- LED azul parpadea en correspondencia con el tiempo de riego.

- Uso de memoria según ESP-IDF:
  IRAM: 40.57% / DRAM: bajo uso / RTC: mínimo

## Referencias

[1] Quiroga, D. (2025). Sistemas embebidos (Séptimo semestre IMEC, 2025–1S). 
    Universidad Tecnológica del Uruguay, Instituto Tecnológico Regional Suroeste.

