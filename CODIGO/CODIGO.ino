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
bool estado_stop = false;
bool borrar_pantalla = false;
int i=0;
float temperatura = 0;
typedef enum{
  ESPERA,
  CALENTANDO,
  ENFRIANDO,
  STOP
}estados;
estados soldadora = ESPERA;
byte customChar[] = {
  B01110,
  B10001,
  B10001,
  B10001,
  B01110,
  B00000,
  B00000,
  B00000
};
//const int outputPin   = 10;
//bool funcionamiento = true;
//int setpoint=260;

void setup()
{
  Serial.begin(9600);
  pinMode(LED_PILOTO, OUTPUT);
  digitalWrite(LED_PILOTO, LOW);
  pinMode(RELE_PLANCHA, OUTPUT);
  digitalWrite(RELE_PLANCHA, LOW);
  pinMode(BOTON_INICIO, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOTON_INICIO), iniciar_calentado, FALLING);
  pinMode(BOTON_STOP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOTON_STOP), parada_emergencia, FALLING);
  lcd.init();
  lcd.createChar(0, customChar);
  lcd.backlight();
  lcd.setCursor(5,1);
  lcd.print("Bienvenido");
  delay(1500);
  lcd.clear();
  /*pinMode(outputPin, OUTPUT);
  pid.begin();
  pid.setpoint(setpoint);
  pid.tune(1, 1, 1);
  pid.limit(0, 255);*/
}

void loop()
{
  switch (soldadora)
  {
    case ESPERA:
    estado_espera = true;
    mostrar_mensaje_2();
    delay(300);
    break;
    case CALENTANDO:
    temperatura = leer_termopar();
    mostrar_mensaje();
    /*if (temperatura>30)
    {
      soldadora=ENFRIANDO;
    }*/
    if (i==3)
    {
      soldadora = ENFRIANDO;
      borrar_pantalla=true;
    }
    i++;
    delay(300);
    break;
    case ENFRIANDO:
    temperatura = leer_termopar();
    if (i<=6)
    {
      digitalWrite(RELE_PLANCHA, LOW);
      mostrar_mensaje();
      if (i==6)
      {
        borrar_pantalla=true;
      }
    }
    else
    {
      mostrar_mensaje_2();
      digitalWrite(LED_PILOTO, HIGH);
    }
    i++;
    if (i==15)
    {
      soldadora = ESPERA;
      digitalWrite(LED_PILOTO, LOW);
      borrar_pantalla=true;
      i=0;
    }
    delay(300);
    break;
    case STOP:
    estado_espera=false;
    if (borrar_pantalla)
    {
      lcd.clear();
      borrar_pantalla=false;
    }
    lcd.setCursor(1, 1);
    lcd.print("PROGRAMA  DETENIDO");
    lcd.setCursor(0, 2);
    lcd.print("REINICIE EL PROGRAMA");
    delay(300);
    break;
  }
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
      estado_espera=false;
      borrar_pantalla=true;
    }
    tiempo = millis();
  }
}

void parada_emergencia()
{
  if (estado_stop == false)
  {
    if (millis()-tiempo > tiempo_limite)
    {
      soldadora = STOP;
      digitalWrite(RELE_PLANCHA, LOW);
      borrar_pantalla=true;
      estado_stop=true;
    }
    tiempo = millis();
  }
}

void mostrar_mensaje()
{
  if (borrar_pantalla)
  {
    lcd.clear();
    borrar_pantalla = false;
  }
  lcd.setCursor(5, 1);
  if (soldadora == CALENTANDO)
  {
    lcd.print("CALENTANDO");
  }
  else if (soldadora==ENFRIANDO)
  {
    lcd.print("ENFRIANDO!");
  }
  mostrar_temperatura();
}

void mostrar_mensaje_2()
{
  if (borrar_pantalla)
  {
    lcd.clear();
    borrar_pantalla=false;
  }
  if (soldadora == ESPERA)
  {
    lcd.setCursor(3,1);
    lcd.print("PULSE EL BOTON");
    lcd.setCursor(4,2);
    lcd.print("PARA INICIAR");
  }
  else if (soldadora == ENFRIANDO)
  {
    lcd.setCursor(2, 1);
    lcd.print("RETIRAR LA PLACA");
    mostrar_temperatura();
  }
}

void mostrar_temperatura()
{
  lcd.setCursor(6, 2);
  lcd.print(temperatura, 2);
  lcd.setCursor(12, 2);
  lcd.write(0);
  lcd.setCursor(13, 2);
  lcd.print("C");
}