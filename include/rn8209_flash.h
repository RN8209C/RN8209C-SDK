#ifndef __RN8209_FLASH_H
#define __RN8209_FLASH_H

#define HAVE_INIT       0X30
#include "rn8209c_u.h"
struct rn8209c_flash
{
    uint8_t init;
    STU_8209C param;
};

extern struct rn8209c_flash stu8209c_flash;
void read_rn8209_param();
void write_rn8209_param();
#endif