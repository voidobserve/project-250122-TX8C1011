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

/*
	当前检测是否充电的ad，使用外部3.3K上拉，2.2K下拉，
	分压系数 == 2.2 / (2.2 + 3.3)
	为 0.4
*/
#define ADCDETECT_CHARING_THRESHOLD 2048 // 检测是否充电的adc值
 
// 充电时，检测到电池满电时对应的ad值
#define ADCDETECT_BAT_FULL (3557) //  8.4V对应的ad值
#define ADCDETECT_BAT_NULL_EX (280)
  

// 快满电时对应的电压ad值，现在使用8.1V
#define ADCVAL_NEAR_FULL_CHAGE (3430)

void charge_scan_handle(void);

#endif // 
