===========================================

Version: V1_01

Author:  raymonxiu

Date:     2012-1-19 13:46:07

Description:

newest module list:(X = 0 or 1)
insmod sun4i_csiX.ko ccm="ov7670" i2c_addr=0x42
insmod sun4i_csiX.ko ccm="gc0308" i2c_addr=0x42
insmod sun4i_csiX.ko ccm="gt2005" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="hi704"  i2c_addr=0x60
insmod sun4i_csiX.ko ccm="sp0838" i2c_addr=0x30
insmod sun4i_csiX.ko ccm="mt9m112" i2c_addr=0xba
insmod sun4i_csiX.ko ccm="mt9m113" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="ov2655" i2c_addr=0x60
insmod sun4i_csiX.ko ccm="hi253" i2c_addr=0x40
insmod sun4i_csiX.ko ccm="gc0307" i2c_addr=0x42
insmod sun4i_csiX.ko ccm="mt9d112" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="ov5640" i2c_addr=0x78

V1_01
CSI: Fix bugs, add new modules support and modity power/standby interface
     Modify new camera modules default menuconfig
1) Fix bugs for CTS test
2) Fix bugs for crash when insmod after rmmod
3) Add default format for csi driver
4) Modify the power on/off,stanby on/off interface
5) Fix bugs for multipex dual sensors using one csi
6) Add gc0307, mt9d112 and ov5640 modules support
7) Fix gc0308 AWB alternation bug
8) Modify new camera modules default menuconfig

V1_00
CSI:Initial version for linux 3.0.8
1) Ported from A10 V1_02


