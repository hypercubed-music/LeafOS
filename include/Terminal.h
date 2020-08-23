#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#ifndef terminal_h
#define terminal_h

class Terminal : public Print {
  public:
    Terminal() {
      memset(buffer, 0, sizeof(buffer)); //clear buffer
    }
    
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

    void drawTerm(Adafruit_ST7735 tft_out) {
      tft_out.setCursor(0,0);
      for (unsigned int i = screenY; i < screenY + 16; i++) {
        if (!screenBuffer[i-screenY].equals(buffer[i])) {
          if (buffer[i].length() != 0) {
            screenBuffer[i-screenY] = buffer[i];
            tft_out.println(buffer[i].substring(screenX, (screenX + 21 < buffer[i].length() ? screenX + 21 : buffer[i].length())) + "                     ");
          } else {
            tft_out.println("                     ");
          }
          screenBuffer[i-screenY] = buffer[i];
        } else {
          tft_out.println();
        }
      }
    }

    void showTerm(Adafruit_ST7735 tft_out) {
      tft_out.fillScreen(ST77XX_BLACK);
      drawTerm(tft_out);
    }

    void hideTerm(Adafruit_ST7735 tft_out) {
      tft_out.fillScreen(ST77XX_BLACK);
    }

    void clearTerm(Adafruit_ST7735 tft_out) {
      memset(buffer, 0, sizeof(buffer)); //clear buffer
      cursorX = 0;
      cursorY = 0;
      screenX = 0;
      screenY = 0;
      drawTerm(tft_out);
    }
  private:
    String buffer[2048];
    String screenBuffer[16];
    unsigned int cursorX = 0;
    unsigned int cursorY = 0;
    unsigned int bufsize;
    unsigned int screenX = 0;
    unsigned int screenY = 0;
};

#endif