#include <Arduino.h>
#include <Wire.h>
#include <LuaArduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <SPI.h>
#include <lua_include.h>
//#include <lua/lapi.h>

//#define LUA_EXTRALIBS {"arduino", luaopen_arduino};

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
String buffer[2048];
String screenBuffer[16];
unsigned int cursorX = 0;
unsigned int cursorY = 0;
unsigned int bufsize;
unsigned int screenX = 0;
unsigned int screenY = 0;
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

class Terminal : public Print {
  public:
    size_t write(uint8_t a) {
      if (a == '\b') {
        if (buffer[cursorY].length() == 0 && cursorY > 0) {
          for (unsigned int i = cursorY; i < bufsize; i++) {
            buffer[i] = buffer[i+1];
          }
          cursorY--;
          bufsize--;
          cursorX = buffer[cursorY].length();
          if (cursorY < screenY + 3) {
            screenY--;
          }
        } else {
          buffer[cursorY] = buffer[cursorY].substring(0, cursorX-1) + buffer[cursorY].substring(cursorX, buffer[cursorY].length());
          cursorX--;
        }
      } else if (a == '\n') {
        //tft.drawRect(6*(cursorX-screenX)-1, 8*(cursorY-screenY)-1, 4, 8, ST7735_BLACK);
        cursorY++;
        bufsize++;
        cursorX = 0;
        for (unsigned int i = bufsize + 1; i > cursorY; i--) {
          buffer[i] = buffer[i-1];
        }
      } else {
        buffer[cursorY] = buffer[cursorY].substring(0, cursorX) + (char)a + buffer[cursorY].substring(cursorX, buffer[cursorY].length());
        cursorX++;
      }
      
      screenX = (cursorX > 21) ? cursorX - 21 : 0;
      if (cursorY > screenY + 15) {
        screenY = cursorY - 15;
      } else if (cursorY < screenY) {
        screenY = cursorY;
      }
      return 1;
    }

    size_t write(const uint8_t *buffer, size_t size) {
      size_t n = 0;
      while (size--) {
        if (write(*buffer++)) n++;
        else break;
      }
      //drawTerm();
      return n;
    }

    size_t write(const char *buffer, size_t size) {
      return write((const uint8_t *)buffer, size);
    }

    size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
    }

    size_t print(const __FlashStringHelper *ifsh)
    {
      PGM_P p = reinterpret_cast<PGM_P>(ifsh);
      size_t n = 0;
      while (1) {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0) break;
        if (write(c)) n++;
        else break;
      }
      drawTerm();
      return n;
    }

    size_t print(const String &s)
    {
      write(s.c_str(), s.length());
      return 0;
    }

    size_t print(const char str[])
    {
      write(str);
      return 0;
    }

    size_t print(char c)
    {
      write(c);
      return 0;
    }

    size_t println(void)
    {
      write("\n");
      return 0;
    }

    size_t println(const String &s)
    {
      print(s);
      println();
      return 0;
    }

    size_t println(const char c[])
    {
      print(c);
      println();
      return 0;
    }

    size_t println(char c)
    {
      print(c);
      println();
      return 0;
    }

  private:
    void drawTerm() {
      tft.setCursor(0,0);
      for (int i = screenY; i < screenY + 16; i++) {
        if (!screenBuffer[i-screenY].equals(buffer[i])) {
          if (buffer[i].length() != 0) {
            screenBuffer[i-screenY] = buffer[i];
            tft.println(buffer[i].substring(screenX, (screenX + 21 < buffer[i].length() ? screenX + 21 : buffer[i].length())) + "                     ");
          } else {
            tft.println("                     ");
          }
        } else {
          tft.println();
        }
      }
      //Draw cursor
      //tft.fillRect(6*(cursorX-screenX), 8*(cursorY-screenY), 2, 7, ST7735_WHITE);
    }
};

Terminal term;
void drawTerm() {
  tft.setCursor(0,0);
  for (int i = screenY; i < screenY + 16; i++) {
    if (!screenBuffer[i-screenY].equals(buffer[i])) {
      if (buffer[i].length() != 0) {
        screenBuffer[i-screenY] = buffer[i];
        tft.println(buffer[i].substring(screenX, (screenX + 21 < buffer[i].length() ? screenX + 21 : buffer[i].length())) + "                     ");
      } else {
        tft.println("                     ");
      }
    } else {
      tft.println();
    }
  }
  //tft.fillRect(6*(cursorX-screenX) + 1, 8*(cursorY-screenY), 2, 7, ST7735_WHITE);
}

void setup() {
  asm(".global _printf_float");
  Serial1.begin(115200);
  memset(buffer, 0, sizeof(buffer)); //clear buffer
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
  drawTerm();
  Serial.println(OUTPUT);
  Serial.println(LED_BUILTIN);
}

void loop() {
  while(1) {
    inputBuffer = "";
    term.print(">");
    drawTerm();
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
        drawTerm();
      }
    }
    term.println();
    inByte = 0;
    lua->loadString(inputBuffer);
  }
  delay(50);
}
