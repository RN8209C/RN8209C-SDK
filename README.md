# RN8209C-SDK
MOKO provides power metering SDK based on RN8209C chip

## note
Please use the SDK according to the porting interface to avoid usage problems

## Introduction of provided files
The provided SDK includes two parts, one part is the driver and calibration method of RN8209C, another is the calibration and application method based on MOKO hardware and software.

1.	 The “src “ directory stores .c files, the “include” directory stores .h files. The “main.c” is an example document used to explain how to use this SDK based on MK114 and MK117 hardware.
2.	 “rn8209c_u.c “ file includes RN8209C driver and calibration method.
3.	“rn8209c_user.c” file includes RN8209C port driver function, calibration function and reading energy measuring data function. MK114 and MK117 hardware users can realize the whole development with this file.
4.	“rn8209_flash.c” file used to read and save the calibration parameters of MK114 and MK117.
5.	“cJSON.c” is the JSON format file during the calibration.
6.	“mk114_led.c” is an example file of MK114 led controlling.
7.	“[ws2812.c](https://github.com/RN8209C/RN8209C-SDK/blob/master/src/ws2812.c)” is an example file of MK117 led controlling.
8.	 “[example_prj_52832/rn8209c_52832](https://github.com/RN8209C/RN8209C-SDK/tree/master/example_prj_52832/rn8209c_52832)” is the example project for MK114B and MK117B hardware.


## add nrf52832 example prj
   in dir example_prj_52832 ,have a example prj ,this prj work in nrfsdk 13。 

