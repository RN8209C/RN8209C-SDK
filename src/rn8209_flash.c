#include "nvs.h"
#include "nvs_flash.h"
#include "rn8209_flash.h"
#include "rn8209c_user.h"
struct rn8209c_flash stu8209c_flash;

void read_rn8209_param()
{

    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "user_save");
    assert(partition != NULL);

    static uint8_t read_data[sizeof(struct rn8209c_flash)]; 
    memset(read_data, 0, sizeof(read_data));

    uint8_t  err =0xff;
    esp_partition_read(partition, 0, read_data, sizeof(read_data));
    memcpy((uint8_t *)&stu8209c_flash,read_data,sizeof(struct rn8209c_flash));
    if(stu8209c_flash.init != HAVE_INIT)
    {
        memset((uint8_t *)&stu8209c_flash,0,sizeof(struct rn8209c_flash));
        stu8209c_flash.param.Ku =  18570;
        stu8209c_flash.param.Kia = 136702;
    }
    set_user_param(stu8209c_flash.param);
}
void write_rn8209_param()
{
       uint8_t err=0xff;
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "user_save");
        assert(partition != NULL);

    // step 2: 
    err=esp_partition_erase_range(partition, 0, partition->size);
    if(err!=0)
   {
   	 printf("user  flash erase range ----%d",err);
   }
    stu8209c_flash.init = HAVE_INIT;
    err = esp_partition_write(partition, 0,(uint8_t *)&stu8209c_flash,sizeof(stu8209c_flash));
    if(err!=0)
   {
   	 printf( "user  flash write datas ----2222%d",err);
   } 
}
