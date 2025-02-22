# Enable debug with nucleo-stm32h563

Use the ST-modified openocd.
First, launch gdbserver through openocd.
```
openocd -f board/st_nucleo_h563zi.cfg
```

WIP: This is annoying but I can't connect directly with gdb
I need to reset the target first using telnet as a workaround.
```
telnet localhost 4444
reset
exit
```

Then you can run arm-none-eabi-gdb
```
arm-none-eabi-gdb <path_to_elf>
target remote :3333
```
