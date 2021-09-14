#ifndef byte_fifo_h____
#define byte_fifo_h____


#include "stdint.h"
#include "stdbool.h"
#include "string.h"


typedef struct
{
uint16_t in;      
uint16_t out;   
uint16_t max_buf;
uint8_t *buf;
}STU_BYTE_QUEUE;


bool queue_byte_out(STU_BYTE_QUEUE *Queue,uint8_t *pdata);
bool queue_byte_in(STU_BYTE_QUEUE *Queue,uint8_t datain);
bool queue_byte_is_empty(STU_BYTE_QUEUE *Queue);
void queue_byte_init(STU_BYTE_QUEUE *Queue,uint8_t *pbuf,uint16_t buf_len);


void queue_buf_write(STU_BYTE_QUEUE *Queue,uint8_t *buf,uint16_t buflen);
uint16_t queue_buf_read(STU_BYTE_QUEUE *Queue,uint8_t *buf,uint16_t read_len);
uint16_t queue_bytes(STU_BYTE_QUEUE *Queue);


#endif



