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


#include "rn8209_flash.h"
#include "rn8209c_user.h"
struct rn8209c_flash stu8209c_flash;

// if no param, init once
static void clear_rn8209_param(void)
{
	memset(&stu8209c_flash,0,sizeof(struct rn8209c_flash));
	stu8209c_flash.init = HAVE_INIT;
	stu8209c_flash.param.Ku =  18570;
    stu8209c_flash.param.Kia = 136702;
	
}
void read_rn8209_param(void)
{
	//user write
	// if  read success , call  func set_user_param
	//if read false ,call func clear_rn8209_param and set_user_param
	clear_rn8209_param();
	set_user_param(stu8209c_flash.param);
}
void write_rn8209_param(void)
{
	//user write
	// store calibrate param
}


