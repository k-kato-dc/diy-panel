# diy-panel
For additional status LCD-320x170.

**Disclaimer:** This software was developed for my personal use and is provided “as is,” without warranty of any kind. Use it at your own risk.

**Intended Audience:** Users with experience in C programming and basic hardware assembly. Not a finished consumer product.

## Requirements

This software is a "Do It Yourself" project I created to display additional status information on my PC. Currently, the following types of displays are supported:

- DIY-LCD of Acemagic S1
- pico-lcd-2 and Raspberry Pi Pico
- 320x170 LCD module with ST7789 LCD controller and Raspberry Pi Pico

![Fig1](/optional/fig1.png)

**Supported Operating Systems:**

- Rocky Linux, Alma Linux, MIRACLE LINUX (version 8 or 9)
- Debian 11, Ubuntu 24, etc.
- Windows 10 or later

## Files

The main source code and resources are organized as follows:

- `diy-panel/`: Bitmap display sample
- `optional/bitmap_src`: Bitmap data for diy-panel
- `optional/diy-lcd`: LED control sample
- `optional/diy-pico`: Raspberry Pi Pico module for connecting diy-panel

Status information collector scripts included in the release package:

- `diy-pstat`: Script to check process status
- `diy-dstat`: Script to check performance values

## Installation

Extract the [release module](/releases/latest) and run the install script.

- Modules are placed in `/opt/AYOR/` or `C:\Program Files (x86)\AYOR`
- Modules are invoked every 1 minute by cron (on Linux) or Task Scheduler (on Windows)
- `diy-panel/diy-ctl`: Script to start and stop modules
- `diy-panel/custom`: Script for changing color, rotation, etc.
- `diy-panel/pstat/`: Script for process status monitoring
- `diy-panel/dstat/`: Script for performance monitoring

## Known Problems

- The process is not lightweight on Windows. Unlike on Linux, the command-line shell and OS commands for process and performance monitoring are inefficient. Replacing the C module might be a solution, but I have not done so because it is not a critical issue and the batch scripts can be easily modified on the target PCs.
- Occasionally, the duplicate running check fails. This is just a sample; if the panel module does not start, please delete the `.pid` file in the panel folder.
- The `dstat` and `pstat` scripts must be modified according to your environment. For example, you may need to change the names of network interfaces (NICs), disks, etc.
- `diy-led` does not work on Linux. This may be due to the kernel serial driver specifications, but I have not been able to identify the cause.
