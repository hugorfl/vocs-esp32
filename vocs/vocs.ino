#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <pitches.h>
#include <Tone32.h>

#define DHT_PIN 13
#define DHT_TYPE DHT22

#define MQ_PIN 36

#define LCD_I2C_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#define LED_GREEN_PIN 6
#define LED_YELLOW_PIN 7
#define LED_RED_PIN 8

#define BUZZER_PIN 35
#define BUZZER_CHANNEL 0

#define BTN_ACK_PIN 15

#define REFRESH_WAIT 1000

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd (LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

struct Button {
  uint32_t btnPresses;
  bool isPressed;
};

Button btnAck = {0, false};

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

ICACHE_RAM_ATTR void btn_ack() {
  btnAck.btnPresses++;
  btnAck.isPressed = true;
}

void setup() {
  Serial.begin(115200);
  
  dht.begin();
  lcd.init();
  lcd.backlight();

  // pinMode(BUZZER_PIN, OUTPUT);

  pinMode(BTN_ACK_PIN, INPUT_PULLUP);
  attachInterrupt(BTN_ACK_PIN,  btn_ack, FALLING);
  
}

void loop() {
  lcd.clear();
  
  if (btnAck.isPressed) {
    printAck();
    btnAck.isPressed = false;
  }

  printTemperature(dht.readTemperature());
  printHumidity(dht.readHumidity());

  delay(REFRESH_WAIT);
  
}

void printLCD(int row, String text) {
  lcd.setCursor(0, row);
  lcd.print(text);
}

void printTemperature(float temperature) {
  String text = "Temp.: ";
  text.concat(temperature);
  text.concat((char) 0xDF);
  
  printLCD(0, text);
}

void printHumidity(float humidity) {
  String text = "Humedad: ";
  text.concat(humidity);
  text.concat("%");
  
  printLCD(1, text);
}

void printAck() {
  String text = "Boton ACK presionado ";
  text.concat(btnAck.btnPresses);
  text.concat(" veces");

  printLCD(0, text);
  rotateTextLeft(text);
  soundBuzzer();
  lcd.clear();
}

void rotateTextLeft(String text) {
  int screenRefresh = 600;
  
  for (int i = 0; i < text.length() - LCD_COLS; i++) {
    delay(screenRefresh);
    lcd.scrollDisplayLeft();
  }
  
  delay(screenRefresh);
}

void soundBuzzer() {
  for (int note = 0; note < 8; note++) {
    int noteDuration = 1000 / noteDurations[note];
    tone(BUZZER_PIN, melody[note], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}
