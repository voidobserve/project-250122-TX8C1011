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

	如果用内部 2.4 V参考电压，12位精度（0-4096），
	那么 1单位ad值 相当于电池电池电压：
	0.00236150568181818181818181818182 V
*/

/*
	当前检测是否充电的ad，使用外部3.3K上拉，2.2K下拉，
	分压系数 == 2.2 / (2.2 + 3.3)
	为 0.4
*/
#define ADCDETECT_CHARING_THRESHOLD 2048 // 检测是否充电的adc值

// 充电时，检测到电池满电时对应的ad值
// #define ADCDETECT_BAT_FULL (3557) //  8.4V对应的ad值（实际测得在8.30V）
#define ADCDETECT_BAT_FULL (3599) //  8.5V对应的ad值（实际测得在8.38V）
// #define ADCDETECT_BAT_FULL (3620) // 实际测得在8.38V，会触发电池的过充保护）
#define ADCDETECT_BAT_NULL_EX (280)

// 快满电时对应的电压ad值，现在使用8.1V
#define ADCVAL_NEAR_FULL_CHAGE (3430)

// #define TMP_BAT_VAL_FIX (0)
// typedef struct
// {
// 	u16 adc_bat_val;
// 	u8 bat_val_fix;
// } bat_val_fix_t;

// const bat_val_fix_t bat_val_fix_table[] = {

// 	// { 2619, TMP_BAT_VAL_FIX + 37 },  // 6.0V
// 	{2752, 250}, // 6.5V
// 	{2964, 145}, // 7.0V
// 	{3091, 102}, // 7.3V
// 	{3227, 60},	 // 7.62V
// 	// {4096, 12},	 // 其他电压值
// };

// // 补偿值对照表
// const u8 fix_val_table[]= {
// 	250,
// 	145,
// 	102,
// 	70,
// 	60,
// };

// // 电池电压对照表
// const u16 bat_val_table[] ={
// 	2752,
// 	2964,
// 	3091,
// 	3172,
// 	3227,
// };

void charge_scan_handle(void);

#endif //


#if 0 // 充电电流控制
// if (flag_is_in_charging && 0 == flag_bat_is_full)
if (flag_is_in_charging)
{

#if 1 // 使用计算的方式来控制充电电流

	u8 i = 0; // 循环计数值
	// u8 max_pwm_val = 0;  // 临时存放最大占空比对应的值
	// u8 last_pwm_val = 0; // 记录之前的pwm占空比的值
	const u8 max_pwm_val = TMR2_PRL + 1;  // 临时存放最大占空比对应的值
	volatile u8 last_pwm_val = TMR2_PWML; // 记录之前的pwm占空比的值
	volatile u16 tmp_val = 0;			  // 临时存放需要调节的占空比对应的值
	volatile u16 tmp_bat_val;			  // 存放检测到的电池电压+计算的压差对应的adc值(可以不初始化)
	static volatile u16 tmp_val_l[8] = {0};
	static volatile u8 tmp_val_cnt = 0;

	// last_pwm_val = (u16)TMR2_PWML + ((u16)TMR2_PWMH << 7); // 读出上一次PWM占空比对应的值
	// max_pwm_val = TMR2_PRL + ((u16)TMR2_PRH << 7) + 1;     // 读出PWM占空比设定的、最大的值
	// last_pwm_val = TMR2_PWML;
	// max_pwm_val = TMR2_PRL + 1;

	/*
		修改电压差值，电压差值 = 203 - (adc_bat_val * 122 / 1000)

		推导过程：
		在充电时测得，充电电流1.1A左右，压差为-30(ad值)时，电池一侧电压为7.8V(ad值：1917)
					 充电电流1.1A左右，压差为0(ad值)时，电池一侧电压为6.8V(ad值：1671)
		假设x轴为电压对应的ad值，y轴为压差对应的ad值，建立参考坐标系
		根据这两个测试点，发现x轴正向增长，y轴负向增长，画出的斜线向下，斜率为负，求出斜率
			k = Δy/Δx = (0 - 30) / (1917 - 1671)，约为 -0.122
		建立公式：y = kx + b，代入，解得 b 约为 203 （四舍五入是204）
		y = kx + b ==> 压差 = -0.122 * 充电时的电池电压 + 203
		转换成单片机可以计算的形式：压差 = 203 - (充电时的电池电压 * 122 / 1000)
	*/

	if (adc_bat_val <= 2752) // 如果检测电池电压小于 6.5V
	{
		tmp_bat_val = (adc_bat_val + 37);
	}
	else if (adc_bat_val <= 2964) // 如果检测电池电压小于 7.0V
	{
		// tmp_bat_val = (adc_bat_val + 18);
		tmp_bat_val = (adc_bat_val + 27);
	}
	else if (adc_bat_val <= 3091) // 如果检测电池电压小于 7.3V
	{
		// tmp_bat_val = (adc_bat_val + 9);
		tmp_bat_val = (adc_bat_val + 16);
	}
	else if (adc_bat_val <= 3227) // 如果检测电池电压小于 7.62V
	{
		// tmp_bat_val = (adc_bat_val + 0);
		tmp_bat_val = (adc_bat_val + 0);
		// tmp_bat_val = (u32)adc_bat_val + (294 - (u32)adc_bat_val * 157 / 1000);
	}
	else // 如果在充电时检测到电池电压大于
	{
		// tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 260); // 实际的充电电流更小了 0.75-0.85
		// tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 304); // 1.3-1.5
		// tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 284); // 1.1A-1.2A (电池没电时在0.9-1.2浮动，电池8V时在0.99·1.08浮动)
		// tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 274); // 0.3(刚开始，几分钟后会升到0.9)-0.9

		// 如果检测电池的分压电阻是 22K / 100K，1.2-1.3A,最常见是在1.22A、1.26A
		// 如果检测电池的分压电阻是 220K / 1M，充电电流在0.9A-1A
		// tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 294);
		tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 522);
	}

	tmp_bat_val += 65; // 1.1

	/*
		升压公式：Vo = Vi / (1 - D)

		通过PWM来控制升压，这里假设当前PWM占空比寄存器的值 为 D，PWM占空比寄存器可以到的最大的值 为 1
		Vo = Vi / (PWM占空比寄存器可以到的最大的值 - 当前PWM占空比寄存器的值)
		当前PWM占空比越大，Vo也越大，充电的电流也会越大

		(PWM占空比寄存器可以到的最大的值 - 当前PWM占空比寄存器的值) = Vi / Vo
		当前PWM占空比寄存器的值 = PWM占空比寄存器可以到的最大的值 - Vi / Vo

		这里检测到的充电电压的ad值 == USB-C口电压 / 2[上下拉电阻分压] / 参考电压[3V，那么这里就是除以3] * 4096[ad转换精度，12位-->0~4096]
		即，这里检测到的充电电压的ad值 == USB-C口电压 / 2 / 3 * 4096
		检测到的电池电压的ad值 == 电池电压 * 0.18 / 3V参考电压 * 4096 == 电池电压 * 220 / 1220 / 3V参考电压 * 4096
		(电池的分压电阻： 上拉220K，下拉1M，分压系数： 220 / 1220)

		检测充电电压和检测电池电压使用的不是同一个分压系数，要一起运算时，这里将充电电压的ad再 * 2 * 220 / 1220
		即 (adc_charging_val * 22 / 61)

		再代回公式：当前PWM占空比寄存器的值 = PWM占空比寄存器可以到的最大的值 - Vi / Vo
		当前PWM占空比寄存器的值 = PWM占空比寄存器可以到的最大的值 - 充电电压 / 充电时电池两侧的电压
		tmp_val = max_pwm_val - 充电电压 / 充电时电池两侧的电压
		转换成单片机可以计算的形式：
		tmp_val = max_pwm_val - (adc_charging_val * 22 / 61) / tmp_bat_val，但是 max_pwm_val 的值不是1，不符合 Vo = Vi / (1 - D)
		这里要改成 tmp_val = max_pwm_val - max_pwm_val * (adc_charging_val * 22 / 61) / tmp_bat_val
		tmp_val = max_pwm_val - (adc_charging_val * max_pwm_val * 22 / 61) / tmp_bat_val
	*/
	// D = 1 - (Vi / Vo)
	// tmp_val = max_pwm_val - (adc_charging_val * max_pwm_val * 22 / 61) / tmp_bat_val;
	tmp_val = max_pwm_val - (adc_charging_val * max_pwm_val * 94 / 147) / tmp_bat_val;

	if (tmp_val >= max_pwm_val)
	{
		// 如果PWM占空比对应的值 大于 最大占空比对应的值，说明计算溢出（可能是电池电压过小），按0处理
		tmp_val = 0;
	}

	// 滤波操作，一开始tmp_val会很小，采集多次后趋于一个平均值：
	tmp_val_cnt++;
	tmp_val_cnt &= 0x07;
	tmp_val_l[tmp_val_cnt] = (tmp_val_l[tmp_val_cnt] + tmp_val) >> 1;
	tmp_val = 0;
	for (i = 0; i < 8; i++)
	{
		tmp_val += tmp_val_l[i];
	}
	tmp_val >>= 3;

	if (tmp_val > last_pwm_val)
	{
		// last_pwm_val = last_pwm_val + 1;
		last_pwm_val++;
	}
	// else if (tmp_val < last_pwm_val)
	else
	{
		// last_pwm_val = last_pwm_val - 1;
		last_pwm_val--;
	}

	TMR2_PWML = last_pwm_val % 256;
	// TMR2_PWMH = last_pwm_val / 256;  // 最大占空比的值不超过255，为了节省程序空间，可以不用配置高8位的寄存器
	TMR2_PWMH = 0;

	// 充电时，每隔一段时间再调整一次PWM占空比，否则充电电流跳动会很厉害
	delay_ms(100);
#endif // 使用计算的方式来控制充电电流
} // if (flag_is_in_charging)
#endif // 充电电流控制
