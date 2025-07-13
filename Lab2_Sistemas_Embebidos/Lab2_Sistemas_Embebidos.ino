/*
  Lab 1 – Monitor de Humedad y Riego Automático
  Hardware:
   • Sensor de humedad de suelo → A0
   • Bomba (vía relé)         → D7
   • LED de estado            → D13
  MCU: Arduino UNO
*/
#include <Wire.h>
#include <LiquidCrystal.h>

// ==== Parámetros de configuración ====
#define MoisturePin A2   // Pin analógico del sensor
#define PumpPin 3    // Pin digital para controlar el relé
// switches 10, 11, 12, 13
#define Mode_pin 10
#define Period_pin 11
#define IrrigationTime_pin 12
#define MinMoisture_pin 13
#define Measure_Pin 2

long MoistureThreshold = 30; // Umbral de humedad (0–1023)
int IrrigationPeriod = 15; // Cada cuándo tiempo riega (s)
int IrrigationTime = 3; // Tiempo que la válvula está abierta (s)
int samplePeriod = 0; // Periodo de mediciones (ms) 1000
char Mode = '2'; // Modo de uso: "1"=M1, "2"=M2
long currMoisture = 0;

// ==== Variables para el timer ====
volatile uint16_t timer = 0;
volatile bool     flagEvento   = false;
volatile bool closeFlag = false;

unsigned long last_time = millis();

// ==== Configuración LCD ====
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);  // (rs, en, d4, d5, d6, d7)

// Variables para medicion de tiempos
unsigned long t_flagEvento;

// ==== Prototipos ====
void initTimer1();
void handleTimedMode();
void pumpOn();
void pumpOff();
void updateAll_LCD();

// ==== Setup y Loop ====
void setup() {
  Serial.begin(9600);
  while (!Serial) { /* espera el puerto Serial */ }

  pinMode(MoisturePin, INPUT_PULLUP);
  pinMode(Mode_pin, INPUT_PULLUP);
  pinMode(Period_pin, INPUT_PULLUP);
  pinMode(IrrigationTime_pin, INPUT_PULLUP);
  pinMode(MinMoisture_pin, INPUT_PULLUP);
  pinMode(PumpPin, OUTPUT);
  pinMode(Measure_Pin, OUTPUT);

  pumpOff();   // asegurar estado inicial

  // Inicializar el LCD con el número de  columnas y filas del LCD
  lcd.begin(16, 2);
  // Escribimos el Mensaje en el LCD.
  updateAll_LCD();

  initTimer1();
  digitalWrite(Measure_Pin, LOW);
}

void loop() {
  //digitalWrite(Measure_Pin, HIGH);
  /*
  readSwitches();
  unsigned long now = millis();
  if (now - last_time > samplePeriod){ // Mediciones cada menos frecuencia, no tiene sentido mayores frecuencias para este caso
    currMoisture = readMoisture();
    controlIrrigation(currMoisture);
    updateAll_LCD();
    unsigned long finish = millis();
    Serial.print("5. Tiempo de ejecución del ciclo completo (ms): ");
    Serial.println(finish - now);
  }

  //delay(50); // Bajar frecuencia total para no gastar energía de forma innecesaria
  */
  
  //currMoisture = readMoisture();
  //measure_6(currMoisture);
  measure_4();
  

  //digitalWrite(Measure_Pin, LOW);
  //delay(30);
}

void measure_1(){
  digitalWrite(Measure_Pin, HIGH);
  digitalWrite(Measure_Pin, LOW);
}

void measure_2(){
  digitalWrite(Measure_Pin, HIGH);
  analogRead(MoisturePin);
  digitalWrite(Measure_Pin, LOW);
}

void measure_2b(){
  digitalWrite(Measure_Pin, HIGH);
  int hello = readMoisture();
  digitalWrite(Measure_Pin, LOW);
}

void measure_3(){
  digitalWrite(Measure_Pin, HIGH);
  updateAll_LCD();
  digitalWrite(Measure_Pin, LOW);
  delay(30);
}

void measure_3b(){
  digitalWrite(Measure_Pin, HIGH);
  updateLCD();
  digitalWrite(Measure_Pin, LOW);
  delay(5);
}

void measure_5(){
  digitalWrite(Measure_Pin, HIGH);
  
  readSwitches();
  unsigned long now = millis();
  if (now - last_time > samplePeriod){ // Mediciones cada menos frecuencia, no tiene sentido mayores frecuencias para este caso
    currMoisture = readMoisture();
    controlIrrigation(currMoisture);
    updateLCD();
    //unsigned long finish = millis();
    //Serial.print("5. Tiempo de ejecución del ciclo completo (ms): ");
    //Serial.println(finish - now);
  }

  digitalWrite(Measure_Pin, LOW);
  delay(5);
}

void measure_4(){
  digitalWrite(Measure_Pin, HIGH);
  delay(1000);

  digitalWrite(Measure_Pin, LOW);
  delay(3000);
}

void measure_6(int moistureValue){
  digitalWrite(Measure_Pin, HIGH);
  bool check = flagEvento == true;
  digitalWrite(Measure_Pin, LOW);
  digitalWrite(Measure_Pin, HIGH);
  check = moistureValue < MoistureThreshold;
  digitalWrite(Measure_Pin, LOW);
}

// ==== Lectura switches ====
void readSwitches(){
  if (digitalRead(Mode_pin) == LOW){
    switch(Mode) {
      case '1': Mode = '2';
        break;
      case '2': Mode = '1';
        break;
    }
    Serial.print("Mode changed");
  }
  if (digitalRead(Period_pin) == LOW){
    if (IrrigationPeriod < 20){
      IrrigationPeriod = IrrigationPeriod + 5;
    } else {
      IrrigationPeriod = 10;
    }
    Serial.print("Period changed");
  }
  if (digitalRead(IrrigationTime_pin) == LOW){
    if (IrrigationTime < 5){
      IrrigationTime = IrrigationTime + 2;
    } else {
      IrrigationTime = 1;
    }
    Serial.print("IrrigationTime changed");
  }
  if (digitalRead(MinMoisture_pin) == LOW){
    if (MoistureThreshold < 45){
      MoistureThreshold = MoistureThreshold + 15;
    } else {
      MoistureThreshold = 15;
    }
    Serial.print("MinMois changed");
  }
  //delay(300);
}

// ==== Lectura de Sensor ====
long readMoisture() {
  //unsigned long start = micros();
  long value = (analogRead(MoisturePin)/1023)*100;
  //unsigned long finish = micros();
  //Serial.print("2. Delay de la lectura del sensor de humedad (us): ");
  //Serial.println(finish - start);
  //Serial.print("Humedad: ");
  //Serial.println(value);
  return value;
}

// ==== Lógica de Riego ====
void controlIrrigation(int moistureValue) {
  switch (Mode){
    case '1':
      
      if (moistureValue < MoistureThreshold) {
        digitalWrite(PumpPin, HIGH);
        //unsigned long t_pumpOn = micros();
        //Serial.print("Tiempo de ejecución de la rutina de decisión Modo 1 (us): ");
        //Serial.println(t_pumpOn - t_flagEvento);
      }
      else{
        digitalWrite(PumpPin, LOW);
      }
      break;

    case '2':
      if (flagEvento == true) {
        pumpOn();
      } else{
        digitalWrite(PumpPin, LOW);
      }
      break;
  }
}

// ==== Configura Timer1: CTC @1 Hz ====
void initTimer1() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  // Modo CTC: WGM12=1
  TCCR1B |= (1 << WGM12);
  // Prescaler 256: CS12=1
  TCCR1B |= (1 << CS12);
  // OCR1A = (16e6/256)/1Hz – 1 = 62499
  OCR1A = 62499;
  TCNT1  = 0;
  // Habilita interrupción Compare A
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

// ==== ISR: cuenta segundos y dispara evento tras kIntervaloHoras horas ====
ISR(TIMER1_COMPA_vect) {
  timer++;
  //Serial.print("sp: ");
  //Serial.println(timer);
  if (timer >= IrrigationPeriod) {
    flagEvento = true;
    //t_flagEvento = micros();
  }
  if (flagEvento == true){
    //Serial.print("irigation time:");
    //Serial.println(IrrigationTime + IrrigationPeriod);
    if (timer >= IrrigationTime + IrrigationPeriod){
      timer = 0;
      closeFlag = true;
      flagEvento = false;
      //Serial.println("Bomba: ENCENDIDA");
    }
  }
}
// ==== Control de Bomba ====
void pumpOn() {
  digitalWrite(PumpPin, HIGH);
  //unsigned long t_pumpOn = micros();
  //Serial.print("Tiempo de ejecución de la rutina de decisión Modo 2 (us): ");
  //Serial.println(t_pumpOn - t_flagEvento);
  
  if (closeFlag == true){
    pumpOff();
    closeFlag = false;
  }
}

void pumpOff() {
  digitalWrite(PumpPin, LOW);
  Serial.println("Bomba: APAGADA");
}

// ==== Actualización del LCD ====
void updateLCD() {
  // construir la línea: M:<modo> P:<periodo>m HL:<duración>s
  char buf[2];
  lcd.setCursor(2, 0);
  snprintf(buf, sizeof(buf), "%c", Mode);
  lcd.print(buf);
  lcd.setCursor(6, 0);
  snprintf(buf, sizeof(buf), "%i", IrrigationPeriod);
  lcd.print(buf);
  lcd.setCursor(13, 0);
  snprintf(buf, sizeof(buf), "%i", IrrigationTime);
  lcd.print(buf);

  lcd.setCursor(6, 1);
  snprintf(buf, sizeof(buf), "%.02f", MoistureThreshold*100/1023);
  lcd.print(buf);
  lcd.setCursor(12, 1);
  snprintf(buf, sizeof(buf), "%02ld", currMoisture*100/1023);
  lcd.print(buf);
}

void updateAll_LCD() {
  unsigned long start = micros();
  lcd.clear();
  // construir la línea: M:<modo> P:<periodo>m HL:<duración>s
  char raw1[16];
  snprintf(raw1, sizeof(raw1),
         "M:%c,P:%is,HL:%is", //Mode, Period, How Long
         Mode,
         IrrigationPeriod,
         IrrigationTime);
  lcd.print(raw1);

  lcd.setCursor(0, 1);
  char raw2[16];
  if (currMoisture>99){
    currMoisture = 99;
  }
  snprintf(raw2, sizeof(raw2),
         "mMois:%02ld%%,C:%02ld%%",
         MoistureThreshold,
         currMoisture);
  lcd.print(raw2);
  //Serial.print(MoistureThreshold);
  unsigned long finish = micros();
  Serial.print("3.Tiempo de ejecución de escritura de mensajes en la LCD (us):");
  Serial.print(finish - start);
}

