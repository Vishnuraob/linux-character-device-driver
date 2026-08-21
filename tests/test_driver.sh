#!/bin/bash

DEVICE0="/dev/prochardev0"
DEVICE1="/dev/prochardev1"
SYSFS="/sys/class/prochardev_class"

echo "================================"
echo " ProCharDev Driver Test"
echo "================================"

echo
echo "[1] Checking device files"

if [ ! -e "$DEVICE0" ]; then
    echo "ERROR: $DEVICE0 not found"
    exit 1
fi

if [ ! -e "$DEVICE1" ]; then
    echo "ERROR: $DEVICE1 not found"
    exit 1
fi

echo "Device files found"

echo
echo "[2] Checking SysFS"

if [ ! -d "$SYSFS" ]; then
    echo "ERROR: SysFS class not found"
    exit 1
fi

echo "SysFS class found"

echo
echo "[3] Checking buffer size"

SIZE=$(cat "$SYSFS/prochardev0/buffer_size")

if [ "$SIZE" != "256" ]; then
    echo "ERROR: Unexpected buffer size: $SIZE"
    exit 1
fi

echo "Buffer size: $SIZE bytes"

echo
echo "[4] Checking device 0"

USED0=$(cat "$SYSFS/prochardev0/buffer_used")

echo "Device 0 buffer usage: $USED0 bytes"

echo
echo "[5] Checking device 1"

USED1=$(cat "$SYSFS/prochardev1/buffer_used")

echo "Device 1 buffer usage: $USED1 bytes"

echo
echo "[6] Checking kernel thread"

if ! sudo dmesg | grep -q "prochardev0: kernel thread started"; then
    echo "WARNING: Device 0 kernel thread message not found"
else
    echo "Device 0 kernel thread is running"
fi

if ! sudo dmesg | grep -q "prochardev1: kernel thread started"; then
    echo "WARNING: Device 1 kernel thread message not found"
else
    echo "Device 1 kernel thread is running"
fi

echo
echo "================================"
echo " All basic checks completed"
echo "================================"
