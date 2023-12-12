#ifndef __SEM_51822_H___
#define __SEM_51822_H___

#include "stdint.h"
#include "stdbool.h"
typedef enum
{
    NO_SEM=0,			//0


} ENUM_SEM;


void SemEmpty(void);
void SendSem(ENUM_SEM Sem);
ENUM_SEM GetSem(void);




#endif



