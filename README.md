# LCD

This repository contains 2 drivers for controlling the LCD RGB 1602 sensor v1.0 from gravity. <br>
It is composed of : <br>
    - AIP31668L chip as char controller. <br>
    - PCA9633 LCD chip for backlight control. <br>
As this LCD module is composed of two chip on the I2C line, I've created a separate drivers for each.  <br>

The DTS entry for theses components are located inside a i2c node. <br>
It include the compatible = "mine,device." line. According to https://docs.zephyrproject.org/latest/build/dts/intro-syntax-structure.html#dt-important-props 
it is used to find the right driver for the node.  <br>
I've also created yaml bindings in ./dts/binding/ <br>
With DTS entry as follow : <br>
```(dts)
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
And the correspondings yaml bindings. <br>

## Setup
I've connected it to 3.3v, GND, SCL:PB8, SDA:PB9 on a nucleo-144 (see MB1404) (nucleo_h563zi) <br>

## Howto
See install note for installation instruction. <br>
Once installed, before running west, user need to : <br>
```(shell)
export BOARD=nucleo_h563zi <br>
source $ZEPHYR_ROOT/.venv/bin/activate
```
Then run 
```(shell)
west build . && west flash
```

## Devices discovery with Hydrabus

To get started I've used the Hydrabus tool. <br>

### LCD RGB Backlight : PCA9633

Send this to enable backlight. <br>
```(shell)
# Hydrabus commands
[0xC0 0x00 0x00]
[0xC0 0x01 0x00]
[0xC0 0x08 0xff]
[0xC0 0x02 0xff]
[0xC0 0x04 0xff]
```
The controller for backlight is PCA9633. (https://www.nxp.com/docs/en/data-sheet/PCA9633.pdf)<br>

### Characteres controller : AIP31068L
```(shell)
# Hydrabus commands
[0x7C 0x80 0x28] # Function set (2 rows)
[0x7C 0x80 0x0C] # Display on
[0x7C 0x80 0x01] # Clear: write space everywhere and move cursor to origin position.
[0x7C 0x80 0x06] # Init text direction
[0x7C 0x40 0x42] # Write ASCII 0x42 in current position and move next char.
```

## Install note

This directory is the top of my project with zephyr -> $WORKSPACE <br>
Zephyr root directory -> ZEPHYR_ROOT=$WORKSPACE/zephyr_project <br>

```(shell)
sudo apt install python3-full python-is-python3 python3-pip
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1
```

Install west, for that I needed to create a venv. <br>
```(shell)
python -m venv $ZEPHYR_ROOT/.venv
. $ZEPHYR_ROOT/venv/bin/activate
pip install west
```

Then follow the west install guide. <br>
```(shell)
cd $ZEPHYR_ROOT
west init
west completion bash >> ~/.bashrc # or put in a dedicated rc file
west update
```

Export a CMake package, that is available to CMake in subsequent build, <br>
this way an application can use zephyr related things in CMake. <br>
```(shell)
west zephyr-export
```

Install addtitional python dependency <br>
```(shell)
pip install -r $ZEPHYR_ROOT/zephyr/scripts/requirements.txt
```

## SDK Installation

```(shell)
cd $WORKSPACE
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-x86_64.tar.xz
wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/sha256.sum | shasum --check --ignore-missing
tar xvf zephyr-sdk-0.16.8_linux-x86_64.tar.xz
cd zephyr-sdk-0.16.8
./setup.sh
```

Udev rules
```(shell)
sudo cp $WORKSPACE/zephyr-sdk-0.16.8/sysroots/x86_64-pokysdk-linux/usr/share/openocd/contrib/60-openocd.rules /etc/udev/rules.d
sudo udevadm control --reload
```

# Note on debug with nucleo-stm32h563

Use the ST-modified openocd. <br>
First, launch gdbserver through openocd. <br>
```(shell)
openocd -f board/st_nucleo_h563zi.cfg
```

WIP: This is annoying but I can't connect directly with gdb <br>
I need to reset the target first using telnet as a workaround. <br>
```(shell)
telnet localhost 4444
reset
exit
```

Then you can run arm-none-eabi-gdb <br>
```(shell)
arm-none-eabi-gdb <path_to_elf>
target remote :3333
```
