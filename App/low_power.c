#include "low_power.h"

void low_power(void)
{
    u8 p0_md0_sta;
    u8 p0_md1_sta;
    u8 p1_md0_sta;
    u8 p1_md1_sta; 
    
    p0_md0_sta = P0_MD0;
    p0_md1_sta = P0_MD1;
    p1_md0_sta = P1_MD0;
    p1_md1_sta = P1_MD1;
    
    // 低功耗下不需要的IO设置成模拟输入
    P0_MD0 = 0xFF;
    P0_MD1 = 0xFF;
    P1_MD0 = 0xFF;
    P1_MD1 = 0xFF;
    
    // 配置P14 低功耗唤醒IO
    P1_MD1    &= ~0x03;
    P1_MD1    |=  0x02;
    P1_AF1    |=  0x03;
    P1_PD     |=  0x10;     // P14下拉
    
    WKUP_CON0  = 0x01;      // 使能通道0，上升沿唤醒
    WKPND      = 0xFF;      // 唤醒前清除pending
    LP_CON    |= 0x40;      // 唤醒后Goon
    // 关闭ADC,DACMP等模拟模块
    // ....
    
    __ENTER_SLEEP_MODE;     // 进入SLEEP
    __SLEEP_AFTER_WAKE_UP;  // SLEEP唤醒后

    // 恢复IO的功能
    P0_MD0 = p0_md0_sta;
    P0_MD1 = p0_md1_sta;
    P1_MD0 = p1_md0_sta;
    P1_MD1 = p1_md1_sta;
    
    // user_printf("Sleep out\n");

    // 重新初始化
    // user_init();


    // 如果一直按着开机/模式按键，返回


    // 配置1s的定时器，用于唤醒，
    // 唤醒后，如果没有按下 开机/模式按键，或是没有在充电，回到低功耗

    

}
