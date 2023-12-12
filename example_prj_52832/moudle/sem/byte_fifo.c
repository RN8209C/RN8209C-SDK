#include "byte_fifo.h"





void queue_byte_init(STU_BYTE_QUEUE *Queue,uint8_t *pbuf,uint16_t buf_len)
{
	Queue->in=0;
	Queue->out=0;
	Queue->max_buf=buf_len;
	Queue->buf=pbuf;
}


/*
return : 
true: queue is empty
false: queue have data
*/

bool queue_byte_is_empty(STU_BYTE_QUEUE *Queue)
{
    if(Queue->in==Queue->out)
    return true;
    else
    return false;
}


/*
return : 
true: queue empty
false: queue full
*/

bool queue_byte_in(STU_BYTE_QUEUE *Queue,uint8_t datain)
{   

	bool ret=true;


	if(((Queue->in+1)%Queue->max_buf)==Queue->out)//queue is full
	{
	ret=false;
	}

	Queue->buf[Queue->in] = datain;
	Queue->in++;
	Queue->in%=Queue->max_buf;
    return ret;


}

/*
para:   pdata--------->point out data

return : 
true:have data in queue
false:no data in queue
*/

bool queue_byte_out(STU_BYTE_QUEUE *Queue,uint8_t *pdata)
{
 
    if(Queue->in == Queue->out) 
    {
        return false;
    }
    else
    {
		*pdata = Queue->buf[Queue->out];
		Queue->out++;
		Queue->out%=Queue->max_buf;
      
        return true;
    }
}

//write buf
void queue_buf_write(STU_BYTE_QUEUE *Queue,uint8_t *buf,uint16_t buflen)
{
	uint16_t i;
	for(i=0 ; i<buflen ; i++)
	{
		queue_byte_in(Queue,buf[i]);
	}	
}
uint16_t queue_buf_read(STU_BYTE_QUEUE *Queue,uint8_t *buf,uint16_t read_len)
{
	uint16_t len = 0,i;
	for(i=0 ;i<read_len ; i++)
	{
		if(queue_byte_out(Queue,buf+i)==0)
			break;
		len++;
	}
	return len;
}
uint16_t queue_bytes(STU_BYTE_QUEUE *Queue)
{
uint16_t out,cnt=0;
		out=Queue->out;
		while(Queue->in != out)
		{
		out++;
        out%=Queue->max_buf;
		cnt++;
		}

   return cnt;
}

