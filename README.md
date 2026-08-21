# ProCharDev - Linux Character Device Driver

A Linux kernel character device driver developed to understand and implement
core Embedded Linux driver concepts including character device registration,
circular buffers, synchronization, blocking I/O, non-blocking I/O, poll(),
IOCTL, kernel threads, multiple device instances, and SysFS.

## Features

- Linux character device registration
- Dynamic kernel memory allocation
- Circular buffer for data storage
- Mutex synchronization
- Blocking read support
- Non-blocking read using O_NONBLOCK
- Wait queues
- poll() support
- IOCTL interface
- Kernel thread for periodic data generation
- Multiple character device instances
- SysFS attributes
- User-space test application
- Automated driver test script

## Project Structure

```text
linux-character-device-driver/
|
├── driver/
|   ├── Makefile
|   ├── include/
|   |   ├── prochardev.h
|   |   └── prochardev_ioctl.h
|   |
|   └── src/
|       ├── main.c
|       ├── file_ops.c
|       ├── memory.c
|       ├── thread.c
|       └── sysfs.c
|
├── user_app/
|   ├── include/
|   └── src/
|       ├── main.c
|       └── poll_test.c
|
├── tests/
|   └── test_driver.sh
|
├── docs/
├── images/
├── LICENSE
├── .gitignore
└── README.md

Driver Architecture

                    User Space
                        |
            +-----------+-----------+
            |                       |
       User Application         poll_test
            |                       |
            +-----------+-----------+
                        |
                  /dev/prochardev0
                  /dev/prochardev1
                        |
                        v
                 Character Driver
                        |
              +---------+---------+
              |                   |
          file_ops.c          ioctl()
              |                   |
              v                   v
        Circular Buffer       IOCTL Commands
              |
        +-----+-----+
        |           |
      Mutex      Wait Queue
        |           |
        +-----+-----+
              |
              v
        Kernel Thread
              |
              v
       Generated Data

Driver Components
main.c

Handles driver initialization and cleanup.

It is responsible for:

Allocating device numbers
Creating character devices
Creating the device class
Creating /dev/prochardev0 and /dev/prochardev1
Starting kernel threads
Removing devices during module cleanup
file_ops.c

Implements the character device operations:

open()
read()
write()
release()
ioctl()
poll()

It also connects these operations to the kernel's
file_operations structure.

memory.c

Implements the circular buffer.

The buffer uses:

read_pos
write_pos
data_count

The buffer supports independent read and write positions and wraps around
when the end of the buffer is reached.

thread.c

Creates a kernel thread for each device.

The thread periodically generates data such as:
Device 0 data 0
Device 0 data 1
Device 0 data 2
The generated data is placed into the device's circular buffer.

sysfs.c

Creates SysFS attributes for each device.

Example:

/sys/class/prochardev_class/prochardev0/

Attributes:

buffer_size
buffer_used
Building the Driver

Go to the driver directory:

cd ~/linux-character-device-driver/driver

Build:

make
Clean:

make clean
Loading the Driver

Load the module:

sudo insmod prochardev.ko

Check:

lsmod | grep prochardev

Check the devices:

ls -l /dev/prochardev*

Expected:

/dev/prochardev0
/dev/prochardev1
Removing the Driver
sudo rmmod prochardev
Check kernel messages:

sudo dmesg | tail -30
User Application

Build the application:

cd ~/linux-character-device-driver/user_app
gcc src/main.c -o prochardev_test

Run device 0:

sudo ./prochardev_test 0

Run device 1:

sudo ./prochardev_test 1

The application provides:
1. Write data
2. Read data
3. Clear buffer
4. Get buffer size
5. Get driver version
6. Exit

IOCTL Interface

The driver provides three IOCTL commands.

Clear Buffer
PROCHARDEV_CLEAR_BUFFER

Clears the circular buffer.

Get Buffer Size
PROCHARDEV_GET_BUFFER_SIZE

Returns:

256
Get Driver Version
PROCHARDEV_GET_VERSION

Returns:

1
Blocking Read

A normal read waits when the buffer is empty.
read()
  |
  v
buffer empty
  |
  v
wait
  |
  v
data arrives
  |
  v
read continues
The driver uses a wait queue to put the reading process to sleep.

Non-Blocking Read

The device also supports:

O_NONBLOCK

When no data is available, the read operation returns:

-EAGAIN

instead of waiting.

poll() Support

The driver implements poll() so applications can wait for the device
to become readable.

Example test:

cd ~/linux-character-device-driver/user_app
gcc src/poll_test.c -o poll_test
sudo ./poll_test

The test waits until data becomes available.

Circular Buffer

The driver uses a 256-byte circular buffer.

The buffer maintains:

read_pos
write_pos
data_count
When the write position reaches the end of the buffer, it returns to
position zero.

Kernel Thread

Each device has its own kernel thread.

The thread periodically generates data and places it into the circular
buffer.

Example:

Device 0 data 0
Device 0 data 1
Device 0 data 2

The thread wakes processes waiting on the device's read queue when new
data becomes available.

Multiple Devices

The driver creates two device instances:

/dev/prochardev0
/dev/prochardev1

Each device has its own:

Circular buffer
Mutex
Wait queue
Kernel thread
The driver uses the minor number to identify the selected device.

SysFS

The driver exposes device information through:

/sys/class/prochardev_class/

Check device 0:

ls /sys/class/prochardev_class/prochardev0

Read buffer size:

cat /sys/class/prochardev_class/prochardev0/buffer_size

Read current buffer usage:

cat /sys/class/prochardev_class/prochardev0/buffer_used
Automated Testing

Run:

./tests/test_driver.sh

The test checks:

Device files
SysFS
Buffer size
Buffer usage
Kernel thread status
Useful Commands

Check kernel messages:

sudo dmesg | tail -30

Check loaded module:

lsmod | grep prochardev

Check device nodes:
ls -l /dev/prochardev*

Check SysFS:

ls /sys/class/prochardev_class/
Technologies
C
Linux Kernel
Linux Kernel Modules
Character Device Drivers
Kernel Threads
Mutex
Wait Queues
Circular Buffers
IOCTL
SysFS
poll()
GCC
Make
Git
Learning Objectives
This project was developed to gain practical understanding of Linux kernel
driver development and the interaction between user-space applications
and kernel-space code.

The project demonstrates how a character device can provide a controlled
interface between applications and kernel functionality.

Author

Vishnu


