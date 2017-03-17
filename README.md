Linux Firmware Debug Kit
========================
Linux Firmware Debug Kit(lfdk) a tool that similiar to "RU" tool that broadly used by BIOS engineers.

History
-------
This tool is original developed by Merck Hung, and the project is hosted on sourceforge
https://sourceforge.net/projects/lfdk/

This fork fixes some issues while building against new kernels.

Build
-----
 * sudo apt install libncurses5-dev
 * make
 * sudo insmod bin/lfdd_drv.ko
 * sudo bin/lfdk

Screenshots
-----------
![screenshot1](https://github.com/acelan/lfdk/raw/master/screenshots/Screenshot01.png)
![screenshot2](https://github.com/acelan/lfdk/raw/master/screenshots/Screenshot02.png)
![screenshot3](https://github.com/acelan/lfdk/raw/master/screenshots/Screenshot03.png)
