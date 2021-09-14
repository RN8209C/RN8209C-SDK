#include "sem.h"


#define MAX_SEM 20


ENUM_SEM EnumSem[MAX_SEM]= {NO_SEM};
uint8_t InSem=1;
uint8_t OutSem=0;

void SemEmpty(void)
{
    OutSem=0;
    InSem=0;
}

void SendSem(ENUM_SEM Sem)
{
    EnumSem[InSem++] =Sem;
    InSem%=MAX_SEM;
}

ENUM_SEM GetSem(void)
{
    ENUM_SEM RetSem;
    if(OutSem!=InSem)
    {
        RetSem = EnumSem[OutSem++];
        OutSem%=MAX_SEM;
        return RetSem;
    }
    return NO_SEM;
}



