# ProCharDev

### Professional Linux Character Device Driver

A modular Linux kernel character device driver written in C to demonstrate core Embedded Linux driver-development concepts, including character-device registration, circular buffers, synchronization, blocking and non-blocking I/O, wait queues, `poll()`, IOCTL interfaces, kernel threads, multiple device instances, and SysFS.

---

## Overview

ProCharDev provides a user-space interface through two character devices:

```text
/dev/prochardev0
/dev/prochardev1
```

Each device instance has its own circular buffer, mutex, wait queue, and kernel thread.

The kernel thread periodically generates data and places it into the corresponding circular buffer. User-space applications can read and write data, use IOCTL commands, monitor device readiness with `poll()`, and inspect device information through SysFS.

---

## Features

- Linux character device registration
- Dynamic kernel memory allocation
- Circular buffer implementation
- Mutex-based synchronization
- Blocking `read()`
- Non-blocking I/O using `O_NONBLOCK`
- Wait queues
- `poll()` support
- IOCTL interface
- Kernel threads
- Multiple character device instances
- Independent buffer for each device
- SysFS attributes
- User-space test application
- Automated test script
- Modular driver source structure
- Kernel logging with `printk()`
- Error handling and cleanup paths

---

## Architecture

```text
                         USER SPACE
┌─────────────────────────────────────────────────────┐
│                                                     │
│  User Application            poll_test              │
│       │                         │                   │
│       └──────────────┬──────────┘                   │
│                      │                              │
│                      ▼                              │
│              /dev/prochardev0                       │
│              /dev/prochardev1                       │
│                                                     │
└──────────────────────┬──────────────────────────────┘
                       │
                       │ open/read/write/ioctl/poll
                       ▼
                    KERNEL
┌─────────────────────────────────────────────────────┐
│                                                     │
│              Character Device Driver               │
│                                                     │
│  ┌──────────────┐       ┌───────────────────────┐  │
│  │  file_ops.c  │       │        IOCTL          │  │
│  │              │       │ CLEAR_BUFFER          │  │
│  │ open()       │       │ GET_BUFFER_SIZE       │  │
│  │ read()       │       │ GET_VERSION           │  │
│  │ write()      │       │                       │  │
│  │ release()    │       └───────────────────────┘  │
│  │ poll()       │                                  │
│  └──────┬───────┘                                  │
│         │                                           │
│         ▼                                           │
│  ┌──────────────────────┐                          │
│  │    Circular Buffer   │                          │
│  │ read_pos             │                          │
│  │ write_pos            │                          │
│  │ data_count           │                          │
│  └──────────┬───────────┘                          │
│             │                                      │
│       ┌─────┴─────┐                                │
│       │           │                                │
│       ▼           ▼                                │
│     Mutex     Wait Queue                           │
│       │           │                                │
│       └─────┬─────┘                                │
│             ▼                                      │
│       Kernel Thread                                │
│             │                                      │
│             ▼                                      │
│       Generated Data                               │
│                                                     │
└─────────────────────────────────────────────────────┘
```

---

## Project Structure

```text
linux-character-device-driver/
├── driver/
│   ├── Makefile
│   ├── include/
│   │   ├── prochardev.h
│   │   └── prochardev_ioctl.h
│   └── src/
│       ├── main.c
│       ├── file_ops.c
│       ├── memory.c
│       ├── thread.c
│       └── sysfs.c
├── user_app/
│   ├── include/
│   │   └── .gitkeep
│   └── src/
│       ├── main.c
│       └── poll_test.c
├── tests/
│   └── test_driver.sh
├── docs/
├── images/
├── .gitignore
├── LICENSE
└── README.md
```

---

## Driver Components

### `main.c`

Handles initialization and cleanup, device-number allocation, device creation, SysFS setup, kernel-thread startup, and resource cleanup.

### `file_ops.c`

Implements:

```text
open()
read()
write()
release()
unlocked_ioctl()
poll()
```

The selected device context is stored in `file->private_data`.

### `memory.c`

Implements the 256-byte circular buffer using:

```text
read_pos
write_pos
data_count
```

### `thread.c`

Creates one kernel thread per device. Threads periodically generate messages such as:

```text
Device 0 data 0
Device 0 data 1
Device 0 data 2
```

### `sysfs.c`

Creates:

```text
buffer_size
buffer_used
```

under:

```text
/sys/class/prochardev_class/prochardev0/
/sys/class/prochardev_class/prochardev1/
```

---

## Circular Buffer

The driver uses a 256-byte circular buffer.

```text
BUFFER_SIZE = 256
```

The buffer maintains:

```text
read_pos
write_pos
data_count
```

When either position reaches the end of the buffer it wraps back to zero.

```text
             256-byte buffer
        ┌─────────────────────────┐
        │       stored data       │
        └─────────────────────────┘
          ↑                   ↑
       read_pos            write_pos
```

`data_count` records how many bytes are currently available.

---

## Synchronization

A mutex protects shared buffer state:

```c
mutex_lock(&dev->lock);
```

and:

```c
mutex_unlock(&dev->lock);
```

This protects buffer contents, `read_pos`, `write_pos`, and `data_count` from concurrent access.

---

## Blocking and Non-Blocking I/O

Normal reads block when the buffer is empty:

```text
read()
  ↓
buffer empty
  ↓
wait queue
  ↓
data arrives
  ↓
wake_up_interruptible()
  ↓
read() continues
```

With `O_NONBLOCK`, an empty buffer causes `read()` to return `-EAGAIN` instead of waiting.

Example:

```c
open("/dev/prochardev0", O_RDONLY | O_NONBLOCK);
```

---

## Wait Queues and `poll()`

Each device has a wait queue. New data wakes waiting processes:

```c
wake_up_interruptible(&dev->read_queue);
```

The driver also implements `.poll`, reporting `POLLIN` and `POLLRDNORM` when data is available.

---

## IOCTL Interface

Defined in:

```text
driver/include/prochardev_ioctl.h
```

Available commands:

| Command | Purpose |
|---|---|
| `PROCHARDEV_CLEAR_BUFFER` | Clear the selected device buffer |
| `PROCHARDEV_GET_BUFFER_SIZE` | Return buffer size |
| `PROCHARDEV_GET_VERSION` | Return driver version |

Current values:

```text
Buffer size: 256 bytes
Driver version: 1
```

---

## Kernel Threads

Each device has its own kernel thread.

Threads are created with:

```c
kthread_run()
```

and stopped with:

```c
kthread_stop()
```

The thread periodically generates data and writes it into the device's circular buffer.

```text
Kernel Thread
      ↓
Generate Data
      ↓
Circular Buffer
      ↓
wake_up_interruptible()
      ↓
Waiting Application
```

---

## Multiple Devices

The driver creates:

```text
/dev/prochardev0
/dev/prochardev1
```

Each device has an independent:

- Circular buffer
- Mutex
- Wait queue
- Kernel thread
- Device state

The minor number identifies the device instance.

---

## SysFS

The driver exposes device information through:

```text
/sys/class/prochardev_class/
```

Check the devices:

```bash
ls /sys/class/prochardev_class/
```

Read buffer size:

```bash
cat /sys/class/prochardev_class/prochardev0/buffer_size
```

Read current buffer usage:

```bash
cat /sys/class/prochardev_class/prochardev0/buffer_used
```

---

## Building the Driver

Check the running kernel:

```bash
uname -r
```

Build:

```bash
cd ~/linux-character-device-driver/driver
make
```

Clean:

```bash
make clean
```

The build produces:

```text
prochardev.ko
```

---

## Loading and Removing

Load:

```bash
sudo insmod prochardev.ko
```

Check:

```bash
lsmod | grep prochardev
```

Check kernel messages:

```bash
sudo dmesg | tail -30
```

Check device nodes:

```bash
ls -l /dev/prochardev*
```

Remove:

```bash
sudo rmmod prochardev
```

---

## Basic Testing

Write data:

```bash
echo -n "Hello" | sudo tee /dev/prochardev0
```

For a controlled read:

```bash
sudo dd if=/dev/prochardev0 bs=5 count=1 status=none
```

Because the driver uses blocking reads, `cat /dev/prochardev0` may continue waiting for additional data.

---

## User-Space Application

Build:

```bash
cd ~/linux-character-device-driver/user_app
gcc src/main.c -o prochardev_test
```

Run device 0:

```bash
sudo ./prochardev_test 0
```

Run device 1:

```bash
sudo ./prochardev_test 1
```

The menu provides:

```text
1. Write data
2. Read data
3. Clear buffer
4. Get buffer size
5. Get driver version
6. Exit
```

---

## `poll()` Test

Build:

```bash
cd ~/linux-character-device-driver/user_app
gcc src/poll_test.c -o poll_test
```

Run:

```bash
sudo ./poll_test
```

The test waits until the device becomes readable.

---

## Automated Testing

The repository contains:

```text
tests/test_driver.sh
```

Make it executable:

```bash
chmod +x tests/test_driver.sh
```

Run:

```bash
./tests/test_driver.sh
```

The script checks device files, SysFS, buffer size, buffer usage, and kernel-thread status.

---

## Debugging

View recent driver messages:

```bash
sudo dmesg | tail -30
```

Filter driver messages:

```bash
sudo dmesg | grep prochardev
```

Check the loaded module:

```bash
lsmod | grep prochardev
```

Check device nodes:

```bash
ls -l /dev/prochardev*
```

Check SysFS:

```bash
ls /sys/class/prochardev_class/
```

---

## Technologies Used

| Technology | Purpose |
|---|---|
| C | Driver and user-space implementation |
| Linux Kernel | Driver execution environment |
| Linux Kernel Modules | Loadable driver |
| Character Devices | Kernel/user-space interface |
| Mutex | Synchronization |
| Wait Queues | Blocking I/O |
| Circular Buffer | Kernel data storage |
| IOCTL | Device control |
| Kernel Threads | Periodic data generation |
| SysFS | Device information |
| `poll()` | Readiness notification |
| GCC | Compilation |
| Make | Kernel module build |
| Git | Version control |

---

## Learning Objectives

This project provides practical experience with:

- Linux kernel module development
- Character-device architecture
- Major and minor device numbers
- Linux VFS file operations
- Kernel/user-space data transfer
- Dynamic memory allocation
- Mutex synchronization
- Circular buffers
- Blocking I/O
- Non-blocking I/O
- Wait queues
- `poll()`
- IOCTL interfaces
- Kernel threads
- SysFS
- Multiple device instances
- Kernel debugging
- Git-based development workflow

---

## Development Milestones

```text
Initial Kernel Module
        ↓
Character Device Registration
        ↓
File Operations
        ↓
Dynamic Buffer
        ↓
Mutex Synchronization
        ↓
Modular Source Structure
        ↓
IOCTL Interface
        ↓
Circular Buffer
        ↓
Blocking I/O
        ↓
Wait Queues
        ↓
O_NONBLOCK
        ↓
poll()
        ↓
Kernel Thread
        ↓
Multiple Devices
        ↓
SysFS
        ↓
User Application
        ↓
Automated Testing
```

---

## Future Improvements

Possible extensions include:

- Interrupt-driven data generation
- `epoll()` support
- Configurable buffer size
- Runtime configuration through SysFS
- Device statistics
- Error counters
- Timestamped driver data
- Kernel tracepoints
- Dynamic device creation
- Hardware-backed data source
- Integration with a real embedded peripheral

---

## License

This project is released under the GNU General Public License v2.0.

---

## Author

**Vishnu**

Embedded Linux / Embedded Systems Project
