# Progress note

This directory is the top of my project with zephyr -> $WORKSPACE
Zephyr root directory -> ZEPHYR_ROOT=$WORKSPACE/zephyr_project

```(shell)
sudo apt install python3-full python-is-python3 python3-pip
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1
```

Install west, for that I needed to create a venv.
```(shell)
python -m venv $ZEPHYR_ROOT/.venv
. $ZEPHYR_ROOT/venv/bin/activate
pip install west
```

Then follow the west install guide.
```(shell)
cd $ZEPHYR_ROOT
west init
west completion bash >> ~/.bashrc # or put in a dedicated rc file
west update
```

Export a CMake package, that is available to CMake in subsequent build,
this way an application can use zephyr related things in CMake.
```(shell)
west zephyr-export
```

Install addtitional python dependency
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

