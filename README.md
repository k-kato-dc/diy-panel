# diy-panel
for additional status LCD-320x170

<strong>
[AYOR notice] This software was developed for my personal use and is provided “as is,” without warranty of any kind. Use it at your own risk.
</strong>

## Requirements ##
This software is a kind of "Do It Yourself" project I created to display additional status information on my PC. Currently, the following types of displays are supported:

* DIY-LCD of Acemagic S1
* pico-lcd-2 and Raspberry Pi Pico
* 320x170 LCD module with ST7789 LCD controller and Raspberry Pi Pico

Supported Operating Systems:

* Rocky Linux, Alma Linux, MIRACLE LINUX (version 8 or 9)
* Debian 11, Ununtu 24 etc.
* Windows 10 or lator

## Files ##
  the source codes are,

* diy-panel: bitmap display sample
* optional/bitmap_src: bitmap data for diy-panel
* optional/diy-lcd: led control sample
* optional/diy-pico: Raspberry Pi Pico module to connect diy-panel

And the status information collector scripts are in the release package.

* diy-pstat: script for check process status
* diy-dstat: script for check performance value

## Installation ##
* extract release module and run the install script
* modules are placed /opt/AYOR/ or C:\Program Files(x86)\AYOR
* modules are invoked every 1 minute by crond or Task Scheduler

[EOF]
