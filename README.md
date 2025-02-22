# LCD

## Howto
Before running west user need to :
export BOARD=nucleo_h563zi
source zephyr tools venv, created at the workspace installation.
(See NOTE_install.md)
This folder is placed in ZEPHYR_ROOT directory.
Then run west build . && west flash

## Whatsup

I've the LCD RGB 1602 sensor v1.0 from gravity.
It is composed of :
    - AIP31668L chip as char controller.
    - PCA9633 LCD chip for backlight control.
I've connected it to 3.3v, GND, SCL:PB8, SDA:PB9 on a nucleo-144 (see MB1404) (nucleo_h563zi)

The DTS entry for all theses components are located inside a i2c node.
It include the compatible = "mine,device."
line. According to https://docs.zephyrproject.org/latest/build/dts/intro-syntax-structure.html#dt-important-props
it is used to find the right driver for the node. 
I've also created a binding in a yaml in <thisapp>/dts/binding/mine,device.yaml

As this LCD module is composed of two chip on the I2C line, I've created a separate
drivers for each : 
    - src/pca9633.c
    - src/aip31068l.c
With DTS entry as follow :
```
	aip31068l: aip31068l@3e {
        status = "okay";
		compatible = "mine,aip31068l";
		reg = <0x3e>;
		cursor = <0>;
		blink = <0>;
    };

	pca9633: pca9633@60 {
        status = "okay";
		compatible = "mine,pca9633";
		reg = <0x60>;
    };
```
And the correspondings yaml bindings.

TODO document driver

## LCD RGB Backlight module from Gravity

Send this to enable backlight.
[0xC0 0x00 0x00]
[0xC0 0x01 0x00]
[0xC0 0x08 0xff]
[0xC0 0x02 0xff]
[0xC0 0x04 0xff]
The controller for backlight is PCA9633.
(https://www.nxp.com/docs/en/data-sheet/PCA9633.pdf)
In the dfrobot library it is init with setReg function

## Characteres controller : AIP31068L
[0x7C 0x80 0x28] # Function set (2 rows)
[0x7C 0x80 0x0C] # Display on
[0x7C 0x80 0x01] # Clear: write space everywhere and move cursor to origin position.
[0x7C 0x80 0x06] # Init text direction
[0x7C 0x40 0x42] # Write ASCII 0x42 in current position and move next char.

