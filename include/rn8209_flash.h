/*************************************************************************
*   	Copyright 2019-2021  MOKO TECHNOLOGY LTD
*
*	Licensed under the Apache License, Version 2.0 (the "License");   
*	you may not use this file except in compliance with the License.   
*	You may obtain a copy of the License at  
*
*	http://www.apache.org/licenses/LICENSE-2.0   
*
*	Unless required by applicable law or agreed to in writing, software   
*	distributed under the License is distributed on an "AS IS" BASIS,   
*	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   
*	See the License for the specific language governing permissions and   
*	limitations under the License.
**************************************************************************/


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