#include <Wire.h>
//#include <PIDController.h>
#include <LiquidCrystal_I2C.h>      
LiquidCrystal_I2C lcd(0x27, 20, 4); 

#include <SPI.h>

//PIDController pid;

#define MAX6675_SO 5
#define MAX6675_CS 6
#define MAX6675_SCK 7
#define LED_PILOTO 8
#define RELE_PLANCHA 9
#define BOTON_INICIO 2
#define BOTON_STOP 3
const int tiempo_limite = 40;
long tiempo = 0;
bool estado_espera = true;
typedef enum{
  ESPERA,
  CALENTANDO,
  ENFRIANDO,
  STOP
}estados;
estados soldadora = ESPERA;
//const int outputPin   = 10;
//bool funcionamiento = true;
//int setpoint=260;

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(5,1);
  lcd.print("Bienvenido");
  delay(1500);
  lcd.clear();
  pinMode(LED_PILOTO, OUTPUT);
  digitalWrite(LED_PILOTO, LOW);
  pinMode(RELE_PLANCHA, OUTPUT);
  digitalWrite(RELE_PLANCHA, LOW);
  pinMode(BOTON_INICIO, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOTON_INICIO), iniciar_calentado, FALLING);
  pinMode(BOTON_STOP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOTON_STOP), parada_emergencia, FALLING);
  Serial.begin(9600);
  /*pinMode(outputPin, OUTPUT);
  pid.begin();
  pid.setpoint(setpoint);
  pid.tune(1, 1, 1);
  pid.limit(0, 255);*/
}

void loop()
{
  Serial.println(soldadora);
  switch (soldadora)
  {
    case ESPERA:
    Serial.println("espera");
    lcd.setCursor(4,1);
    lcd.print("PULSE EL BOTON");
    lcd.setCursor(5,2);
    lcd.print("PARA INICIAR");
    Serial.println("espera");
    delay(300);
    break;
    case CALENTANDO:
    Serial.println(soldadora);
    float temperatura = leer_termopar();
    lcd.setCursor(5, 1);
    lcd.print("CALENTANDO");
    lcd.setCursor(7, 2);
    lcd.print(temperatura, 2);
    if (temperatura>29)
    {
      soldadora=ENFRIANDO;
    }
    delay(300);
    break;
    case ENFRIANDO:
    Serial.println(soldadora);
    digitalWrite(RELE_PLANCHA, LOW);
    temperatura = leer_termopar();
    lcd.setCursor(5, 1);
    lcd.print("ENFRIANDO");
    lcd.setCursor(7, 2);
    lcd.print(temperatura, 2);
    if (temperatura<20)
    {
      digitalWrite(LED_PILOTO, HIGH);
      lcd.setCursor(2, 1);
      lcd.print("RETIRAR LA PLACA");
      lcd.setCursor(7, 2);
      lcd.print(temperatura, 2);
    }
    delay(300);
    break;
    case STOP:
    Serial.println(soldadora);
    Serial.println("kjd");
    lcd.setCursor(1, 1);
    lcd.print("PROGRAMA  DETENIDO");
    lcd.setCursor(0, 2);
    lcd.print("REINICIE EL PROGRAMA");
    delay(300);
    break;
  }
  /*float temperatura = leer_termopar();
  lcd.setCursor(2, 1);
  lcd.print("Termopar tipo K");
  lcd.setCursor(7, 2);
  lcd.print(temperatura, 2);
  delay(300);*/
  /*int output = pid.compute(temperatura);
  analogWrite(outputPin, output);
  if (funcionamiento)
  {
    if (temperatura=setpoint)
    {
      delay(10000);
      setpoint=0;
      funcionamiento = false;
    }
  }*/
}

double leer_termopar()
{

  uint16_t v;
  pinMode(MAX6675_CS, OUTPUT);
  pinMode(MAX6675_SO, INPUT);
  pinMode(MAX6675_SCK, OUTPUT);

  digitalWrite(MAX6675_CS, LOW);
  delay(1);

  // Read in 16 bits,
  //  15    = 0 always
  //  14..2 = 0.25 degree counts MSB First
  //  2     = 1 if thermocouple is open circuit
  //  1..0  = uninteresting status

  v = shiftIn(MAX6675_SO, MAX6675_SCK, MSBFIRST);
  v <<= 8;
  v |= shiftIn(MAX6675_SO, MAX6675_SCK, MSBFIRST);

  digitalWrite(MAX6675_CS, HIGH);
  if (v & 0x4)
  {
    // Bit 2 indicates if the thermocouple is disconnected
    return NAN;
  }

  // The lower three bits (0,1,2) are discarded status bits
  v >>= 3;

  // The remaining bits are the number of 0.25 degree (C) counts
  return v * 0.25;
}

void iniciar_calentado()
{
  if (estado_espera)
  {
    if (millis()-tiempo > tiempo_limite)
    {
      soldadora = CALENTANDO;
      digitalWrite(RELE_PLANCHA, HIGH);
      //lcd.clear();
      estado_espera=false;
    }  
  }
}

void parada_emergencia()
{
  if (millis()-tiempo > tiempo_limite)
  {
    //lcd.clear();
    soldadora = STOP;
    digitalWrite(RELE_PLANCHA, LOW);
  }
}