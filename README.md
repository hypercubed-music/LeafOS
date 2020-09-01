# LeafOS
Standalone Arduino-compatible microcontroller development environment

The "OS" (its more like a development environment) allows you to program the board without the need for a PC. The programming language I'm using right now is Lua (from here: LuaArduino) but I might try to integrate some other languages like Forth or uLisp (circuitpython?).

Planned features:

- USB Keyboard input (done)
- Terminal-like interface w/ horizontal and vertical scrolling (done) (might release as separate library)
- Lua interpreter (integrated but not fully featured yet; some Arduino functions are present)
- Ability to load and run programs from an SD card
- Shell-like command line
- GUI for launching programs
- WiFi co-processor for downloading files (maybe a really basic web browser)
- Basic text editor

Right now it runs on the Adafruit Feather M4 Express, with a trinket M0 running a usb coprocessor (https://github.com/gdsports/usbhostcopro),
and a 3.5 inch screen (Adafruit 2050)
