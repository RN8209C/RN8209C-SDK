#ifndef RN8209c_USER_INTERFACE_H____
#define RN8209c_USER_INTERFACE_H____

#include "stdio.h"
#include "string.h"

#include "rn8209c_u.h"

#define 	RN8209_TASK_STACK_SIZE	4096
#define 	RN8209_TASK_PRIO			7
#define	MAC						"123456789012"
#define 	DEV_NAME				"MK114"
#define  FIRMWARE_VERSION		"V1.0.1"
struct rn8209
{
	uint32_t voltage;
	uint32_t current;
	uint32_t power;
};
extern struct rn8209 rn8209_value;


void set_user_param(STU_8209C param);
void rn8209c_process_init(uint8_t cmd);
#endif




