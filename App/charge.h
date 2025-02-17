#ifndef __CHARGE_H
#define __CHARGE_H

#include "my_config.h"
 
/*
    当前检测电池电压使用1M上拉，470K下拉，
    那么检测到电池的电压的分压系数 == 470K / (1M + 470K)
    约为  0.31972789115646258503401360544218

	将原来的检测电池电压的1M上拉、470K下拉，改为
	1M上拉，330K下拉，
	检测电池电压的分压系数 == 330K / (1M + 330K)
	约为 0.24812030075187969924812030075188
*/
#define ADCDETECT_CHARING_THRESHOLD 2048 // 检测是否充电的adc值
#define BAT_FIX_VAL (1773 / 1000)
#ifdef BAT_FIX_VAL
// #define ADCDETECT_BAT_FULL (3720) //  (3720--对应8.52V)
// #define ADCDETECT_BAT_FULL (3666) //  (3666--对应8.4V,但是实际测得在8.51-8.52V才停止)
// #define ADCDETECT_BAT_FULL (3613) // 计算得出是8.27V， 是在3666的基础上减去一定值（补偿）,实际测得是8.36V
// #define ADCDETECT_BAT_FULL (3626) //  实际测试是8.37
// #define ADCDETECT_BAT_FULL (3639) //  实际测试是8.39V
#define ADCDETECT_BAT_FULL (3642) //  实际测试是 8.40V
// #define ADCDETECT_BAT_FULL (3644) //  实际测试是8.41V
// #define ADCDETECT_BAT_FULL (3647) //  实际测试是 8.41V
#define ADCDETECT_BAT_NULL_EX (280)
#define ADCDETECT_BAT_WILL_FULL (3472) // (1958)
#define ADCVAL_REF_BAT_6_0_V (2618)    // (1477)

#else
#define ADCDETECT_BAT_FULL (2098)
// #define ADCDETECT_BAT_FULL (2083)	   // 充电时，检测电池是否充满电的adc值 充电时电池电压+压差==8.40V,实际充到电池实际的电压在8.24V认为满电
// #define ADCDETECT_BAT_FULL (2112)	   // 充电时，检测电池是否充满电的adc值 8.58V (8.42还未检测充满，用电源来模拟--是在8.6V左右检测到充满)
// #define ADCDETECT_BAT_FULL (2075)	   // 充电时，检测电池是否充满电的adc值 8.44V（直接用分压系数来算是8.427845），实际是充到8.28V后，认为满电
#define ADCDETECT_BAT_WILL_FULL (1958) // 电池将要满电的adc值
// #define ADCDETECT_BAT_EMPTY 1352	   // 电池为空，那么充电时只有充电的电压（5.5V电池电压）

// #define ADCVAL_REF_BAT_6_7_V 1647 // 电池电压6.7V对应的ad值
// #define ADCVAL_REF_BAT_6_4_V 1573 // 电池电压6.4V对应的ad值
// #define ADCVAL_REF_BAT_6_5_V 1597 // 电池电压6.5V对应的ad值
// #define ADCVAL_REF_BAT_5_7_V 1403 // 电池电压5.7V对应的ad值 （实际是5.52V左右，电池保护板先断开了，实际上还没有检测到这个值就保护了）
#define ADCVAL_REF_BAT_6_0_V 1477      // 电池电压6.0V对应的ad值

#endif

// 快满电时对应的电压ad值，现在使用8.1V
#define ADCVAL_NEAR_FULL_CHAGE (3430)

void charge_scan_handle(void);

#endif // 
