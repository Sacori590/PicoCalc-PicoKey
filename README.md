# PicoKey

A marvellous way to use a pico calc as a regular keyboard.

## Why ?

One day I had a dream where pico calc could be used as a regular keyboard.

## Installation :

1. Hold the reset button while plugin your picoCalc using the micro Usb Port.
2. When your picoCalc appear, drag and drop the uf2 file available in the release section.

## Setup :

1. Keep your picoCalc off and plug your micro-usb cable tp your computer.
2. Start your picoCalc and start using it as a keyboard, mouse combo.

## Controls :

- Keyboard is a normal one exception to `F10,F1,F2, break`.
- Control mouse with the directional arrow.
- Switch between mouse control and normal arrow key by pressing `F10`.
- Keep left click pressed with `F1`.
- Keep right click pressed with `F2`.
- because mouse clicks are keep pressed to make a normal click you'll have to press twice to do a normal click. So... yes you'll have to spam click four time your mouse click to double click.
- break is map to `ctrl+c`.

## Issues :

Tiny Usb have an issue with unplug replug, when you unplug the device you'll have to restart it. Maybe because I based my code on an old exemple from the library, I saw that other people had same issue on different project.

## Sources
Thanks to ClockWork and Tiny Usb without them this project coul not be created.
I based my code on the `helloWorld` code from Clockwork (available on https://github.com/clockworkpi/PicoCalc) and the `hid_composite` exemple (available on https://github.com/hathach/tinyusb).
