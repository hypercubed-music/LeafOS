#include <Arduino.h>
#include <Wire.h>
#include <LuaArduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <SPI.h>
#include <lua_arduino.h>
#include <Terminal.h>

Adafruit_ST7735 tft = Adafruit_ST7735(5, 9, 6);
#if defined(EXTERNAL_FLASH_USE_QSPI)
  Adafruit_FlashTransport_QSPI flashTransport;
#elif defined(EXTERNAL_FLASH_USE_SPI)
  Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);
#else
  #error No QSPI/SPI flash are defined on your board variant.h !
#endif

Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
Lua* lua;
String inputBuffer;
long millisTimer;
char inByte;

byte kbd_to_ascii(byte key) {
  switch(key) {
    case 0:
      break;
    case 8:
    case 127:
      return '\b';
    case 9:
      return '\t';
    case 13:
      return '\n';
  }
  return key;
}



Terminal term = Terminal();

void setup() {
  asm(".global _printf_float");
  Serial1.begin(115200);
  // put your setup code here, to run once:
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0,0);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.setTextWrap(false); //use horizontal scrolling instead
  
  term.println("LeafOS 0.0.0");
  term.println("Flash: init");
  if (!flash.begin()) {
    term.println("Init error!");
    while(1);
  }
  term.println("Flash: mount");
  if (!fatfs.begin(&flash)) {
    term.println("mount failed!");
    while(1);
  }
  term.println("Flash: ready");
  lua = new Lua;
  lua->setOut(&term);
  luaopen_arduino(lua->getState());
  if (lua && lua->getState()) {
    lua->help();
  } else {
    term.println("Lua Failed To Allocate!");
  }
  term.drawTerm(tft);
  Serial.println(OUTPUT);
  Serial.println(LED_BUILTIN);
}

void loop() {
  while(1) {
    inputBuffer = "";
    term.print(">");
    term.drawTerm(tft);
    while(inByte != 4) {
      if (Serial1.available() > 0) {
        inByte = Serial1.read();
        if (!(kbd_to_ascii(inByte) == '\b' && inputBuffer.length() == 0) && inByte != 4) {
          term.write((char)kbd_to_ascii(inByte));
          if (inByte == 8 || inByte == 127) {
            inputBuffer.remove(inputBuffer.length() - 1);
          } else {
            inputBuffer += (char)kbd_to_ascii(inByte);
          }
        }
        term.drawTerm(tft);
      }
    }
    term.println();
    inByte = 0;
    lua->loadString(inputBuffer);
  }
  delay(50);
}