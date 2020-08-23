#include <Arduino.h>
#include <Wire.h>
#include <LuaArduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <SPI.h>
//#include <lua_arduino.h>
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
Terminal term = Terminal();

void interruptHook(lua_State* L, lua_Debug *ar) {
  if (Serial1.available() > 0) {
    char tmp = Serial1.read();
    if (tmp == 17) {
      //longjmp(place,1);
      lua_sethook(L, interruptHook, LUA_MASKLINE, 0); 
      luaL_error(L, "KeyboardInterrupt");
    }
  }
}

extern "C" {
  void lua_compat_print(const char *s){
    Lua::getOut()->print(s);
    term.drawTerm(tft);
  }
}

static int l_showTerm(lua_State *L) { term.showTerm(tft); return 0;}
static int l_hideTerm(lua_State *L) { term.hideTerm(tft); return 0;}
static int l_clearTerm(lua_State *L) { term.clearTerm(tft); return 0;}
static int l_width(lua_State *L) {lua_pushnumber(L, tft.width()); return 1;}
static int l_height(lua_State *L) {lua_pushnumber(L, tft.height()); return 1;}
static int l_cursorX(lua_State *L) {lua_pushnumber(L, tft.getCursorX()); return 1;}
static int l_cursorY(lua_State *L) {lua_pushnumber(L, tft.getCursorY()); return 1;}
static int l_drawCircle(lua_State *L) {
  int x0 = (int)lua_tonumber(L, 1);
  int y0 = (int)lua_tonumber(L, 2);
  int r = (int)lua_tonumber(L, 3);
  int color = (int)lua_tonumber(L, 4);
  tft.drawCircle(x0, y0, r, color);
  return 0;
}
static int l_drawRect(lua_State *L) {
  int x = (int)lua_tonumber(L, 1);
  int y = (int)lua_tonumber(L, 2);
  int w = (int)lua_tonumber(L, 3);
  int h = (int)lua_tonumber(L, 4);
  int color = (int)lua_tonumber(L, 5);
  tft.drawRect(x, y, w, h, color);
  return 0;
}
static int l_drawRoundRect(lua_State *L) {
  int x = (int)lua_tonumber(L, 1);
  int y = (int)lua_tonumber(L, 2);
  int w = (int)lua_tonumber(L, 3);
  int h = (int)lua_tonumber(L, 4);
  int r = (int)lua_tonumber(L, 5);
  int color = (int)lua_tonumber(L, 6);
  tft.drawRoundRect(x, y, w, h, r, color);
  return 0;
}
static int l_drawTriangle(lua_State *L) {
  int x0 = (int)lua_tonumber(L, 1);
  int y0 = (int)lua_tonumber(L, 2);
  int x1 = (int)lua_tonumber(L, 3);
  int y1 = (int)lua_tonumber(L, 4);
  int x2 = (int)lua_tonumber(L, 5);
  int y2 = (int)lua_tonumber(L, 6);
  int color = (int)lua_tonumber(L, 7);
  tft.drawTriangle(x0,y0,x1,y1,x2,y2,color);
  return 0;
}
static int l_drawLine(lua_State *L) {
  int x0 = (int)lua_tonumber(L, 1);
  int y0 = (int)lua_tonumber(L, 2);
  int x1 = (int)lua_tonumber(L, 3);
  int y1 = (int)lua_tonumber(L, 4);
  int color = (int)lua_tonumber(L, 5);
  tft.drawLine(x0,y0,x1,y1,color);
  return 0;
}
static int l_drawPixel(lua_State *L) {
  int x = (int)lua_tonumber(L, 1);
  int y = (int)lua_tonumber(L, 2);
  int color = (int)lua_tonumber(L, 3);
  tft.drawPixel(x,y,color);
  return 0;
}
static int l_drawHLine(lua_State *L) {
  int x = (int)lua_tonumber(L, 1);
  int y = (int)lua_tonumber(L, 2);
  int w = (int)lua_tonumber(L, 3);
  int color = (int)lua_tonumber(L, 4);
  tft.drawFastHLine(x,y,w,color);
  return 0;
}
static int l_drawVLine(lua_State *L) {
  int x = (int)lua_tonumber(L, 1);
  int y = (int)lua_tonumber(L, 2);
  int h = (int)lua_tonumber(L, 3);
  int color = (int)lua_tonumber(L, 4);
  tft.drawFastVLine(x,y,h,color);
  return 0;
}
static int l_fillCircle(lua_State *L) {
  int x0 = (int)lua_tonumber(L, 1);
  int y0 = (int)lua_tonumber(L, 2);
  int r = (int)lua_tonumber(L, 3);
  int color = (int)lua_tonumber(L, 4);
  tft.fillCircle(x0,y0,r,color);
  return 0;
}
static int l_fillRect(lua_State *L) {
  int x = (int)lua_tonumber(L, 1);
  int y = (int)lua_tonumber(L, 2);
  int w = (int)lua_tonumber(L, 3);
  int h = (int)lua_tonumber(L, 4);
  int color = (int)lua_tonumber(L, 5);
  tft.fillRect(x,y,w,h,color);
  return 0;
}
static int l_fillRoundRect(lua_State *L) {
  int x = (int)lua_tonumber(L, 1);
  int y = (int)lua_tonumber(L, 2);
  int w = (int)lua_tonumber(L, 3);
  int h = (int)lua_tonumber(L, 4);
  int r = (int)lua_tonumber(L, 5);
  int color = (int)lua_tonumber(L, 6);
  tft.fillRoundRect(x,y,w,h,r,color);
  return 0;
}
static int l_fillTriangle(lua_State *L) {
  int x0 = (int)lua_tonumber(L, 1);
  int y0 = (int)lua_tonumber(L, 2);
  int x1 = (int)lua_tonumber(L, 3);
  int y1 = (int)lua_tonumber(L, 4);
  int x2 = (int)lua_tonumber(L, 5);
  int y2 = (int)lua_tonumber(L, 6);
  int color = (int)lua_tonumber(L, 7);
  tft.fillTriangle(x0,y0,x1,y1,x2,y2,color);
  return 0;
}
static int l_fillScreen(lua_State *L) {
  int color = (int)lua_tonumber(L,1);
  tft.fillScreen(color);
  return 0;
}
static int l_textColor(lua_State *L) {
  int color = (int)lua_tonumber(L,1);
  tft.setTextColor(color);
  return 0;
}
static int l_invertScreen(lua_State *L) {
  boolean inv = lua_toboolean(L, 1);
  tft.invertDisplay(inv);
  return 0;
}
static int l_textSize(lua_State *L) {
  int s = (int)lua_tonumber(L, 1);
  tft.setTextSize(s);
  return 0;
}
static int l_setCursor(lua_State *L) {
  int x = (int)lua_tonumber(L, 1);
  int y = (int)lua_tonumber(L, 2);
  tft.setCursor(x,y);
  return 0;
}
static int l_color(lua_State *L) {
  int r = (int)lua_tonumber(L,1);
  int g = (int)lua_tonumber(L,2);
  int b = (int)lua_tonumber(L,3);
  lua_pushnumber(L, (r<<11) | (g<<5) | (b<<11));
  return 1;
}
static int l_color24(lua_State *L) {
  int r = (int)lua_tonumber(L,1);
  int g = (int)lua_tonumber(L,2);
  int b = (int)lua_tonumber(L,3);
  lua_pushnumber(L, ((r / 8) << 11) | ((g / 4) << 5) | (b / 8));
}

int luaopen_graphics(lua_State *L) {
    lua_register(L, "showTerm", l_showTerm);
    lua_register(L, "hideTerm", l_hideTerm);
    lua_register(L, "clear", l_clearTerm);
    lua_register(L, "width",l_width);
    lua_register(L, "height",l_height);
    lua_register(L, "cursorX",l_cursorX);
    lua_register(L, "cursorY",l_cursorY);
    lua_register(L, "drawCircle", l_drawCircle);
    lua_register(L, "drawRect", l_drawRect);
    lua_register(L, "drawRoundRect", l_drawRoundRect);
    lua_register(L, "drawTriangle", l_drawTriangle);
    lua_register(L, "drawLine",l_drawLine);
    lua_register(L, "drawHLine",l_drawHLine);
    lua_register(L, "drawVLine",l_drawVLine);
    lua_register(L, "srawPixel",l_drawPixel);
    lua_register(L, "fillCircle", l_fillCircle);
    lua_register(L, "fillRect", l_fillRect);
    lua_register(L, "fillRoundRect", l_fillRoundRect);
    lua_register(L, "fillTriangle", l_fillTriangle);
    lua_register(L, "fillScreen", l_fillScreen);
    lua_register(L, "textColor", l_textColor);
    lua_register(L, "invertScreen", l_invertScreen);
    lua_register(L, "textSize", l_textSize);
    lua_register(L, "setCursor", l_setCursor);
    lua_register(L, "color", l_color);
    lua_register(L, "color24", l_color24);
}