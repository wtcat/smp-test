#!/bin/sh
qemu-system-arm -S -s -no-reboot -net none -nographic -smp 4 -icount auto -M realview-pbx-a9 -m 256M -kernel build/arm-rtems6-realview_pbx_a9_qemu/smp.elf
