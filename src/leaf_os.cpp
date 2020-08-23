#include <leaf_os.h>

String inputBuffer;
long millisTimer;
char inByte;

enum {
  MODE_TERM,
  MODE_LUA_TERM_IDLE,
  MODE_LUA_TERM_RUN,
  MODE_LUA_TERM_INIT,
  MODE_GUI_INIT,
  MODE_GUI,
  MODE_LUA_RUNFILE,

} mode;
int currentMode = MODE_LUA_TERM_INIT;

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
  // Start Lua
  lua = new Lua;
  lua->setOut(&term);
  luaopen_graphics(lua->getState());
  lua_sethook(lua->getState(), interruptHook, LUA_MASKCOUNT, 100);
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
  switch (currentMode) {
    case MODE_LUA_TERM_INIT:
      inByte = 0;
      inputBuffer = "";
      term.print(">");
      term.drawTerm(tft);
      currentMode = MODE_LUA_TERM_IDLE;
      break;
    case MODE_LUA_TERM_IDLE:
      if (Serial1.available() > 0) {
        inByte = Serial1.read();
        if (inByte == 4) {
          currentMode = MODE_LUA_TERM_RUN;
        } else if (!(kbd_to_ascii(inByte) == '\b' && inputBuffer.length() == 0)) {
          term.write((char)kbd_to_ascii(inByte));
          if (inByte == 8 || inByte == 127) {
            inputBuffer.remove(inputBuffer.length() - 1);
          } else {
            inputBuffer += (char)kbd_to_ascii(inByte);
          }
        }
        term.drawTerm(tft);
      }
      break;
    case MODE_LUA_TERM_RUN:
      term.println();
      lua->loadString(inputBuffer);
      currentMode = MODE_LUA_TERM_INIT;
      break;
    case MODE_GUI_INIT:
      term.hideTerm(tft);

  }
}