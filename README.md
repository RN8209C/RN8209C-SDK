# RN8209C-SDK

MOKO provides power metering SDK based on RN8209C chip

## note

Please use the SDK according to the porting interface to avoid usage problems

## Introduction of provided files

The provided SDK includes two parts, one part is the driver and calibration method of RN8209C, another is the calibration and application method based on MOKO hardware and software.

1. The `src`  directory stores `.c` files and the `include` directory stores `.h` files of `RN8209C`.
2. `rn8209c_u.c ` file includes RN8209C driver and calibration method.
3. `rn8209c_user.c` file includes RN8209C port driver function, calibration function and reading energy measuring data function.
4. `rn8209_flash.c` file used to read and save the calibration parameters of `MK115` ,`MK114 `and `MK117`.
5. `cJSON.c` is the JSON format file during the calibration.
6. “[example_prj_ESP32](https://github.com/RN8209C/RN8209C-SDK/tree/master/example_prj_ESP32)” is the example project for `MK115` ,`MK114 `and `MK117` hardware.
7. “[example_prj_52832](https://github.com/RN8209C/RN8209C-SDK/tree/master/example_prj_52832)” is the example project for `MK114B` and `MK117B` hardware.

## add nrf52832 example prj

   in dir `example_prj_52832` ,have a example prj ,this prj work in nrfsdk 13。

## add ESP32 example prj

1. in dir `example_prj_ESP32` ,have a example prj ,this prj work in ESP-IDF SDK, for `MK115` ,`MK114 `and `MK117` hardware.
2. The corresponding gateway hardware is defined according to the macros in the `dev_cfg.h` file, default defined `MK115` hardware. `MK115` hardware users can realize the whole development of RN8209C with this prj.
