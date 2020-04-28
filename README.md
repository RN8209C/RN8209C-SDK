# RN8209C-SDK
MOKO provides power metering SDK based on RN8209C chip


## Introduction of provided files
The provided SDK includes two parts, one part is the driver and calibration method of RN8209C, another is the calibration and application method based on MOKO hardware and software.

1.	The “src “ directory stores .c files, the “include” directory stores .h files. The “main.c” is an example document used to explain how to use this SDK based on MK114 hardware.

2.	“rn8209c_u.c “ file includes RN8209C driver and calibration method.

3.	“rn8209c_user.c” file includes RN8209C port driver function, calibration function and reading energy measurement data function. MK114 hardware users can realize the whole development with only this file.

4.	“rn8209_flash.c” file used to read and save the calibration parameters of MK114.

5.	“cJSON.c” is the JSON format file during the calibration.

6.	“Led.c” is an example file of the led controlling.

