/**
 ******************************************************************************
 * @file    main.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    01-05-2021
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 * ��Ȩ˵����������
 *
 ******************************************************************************
 */

#include "include.h"
#include "my_config.h"

volatile bit flag_bat_is_empty = 0; // ��־λ�����ڼ���Ƿ�γ��˵��
volatile bit flag_bat_is_full = 0;  // ����Ƿ񱻳�����ı�־λ

volatile bit flag_is_in_charging = 0;             // �Ƿ��ڳ��ı�־λ
volatile bit flag_tim_scan_maybe_not_charge = 0;  // ���ڸ���ʱ��ɨ��ı�־λ�����ܼ�⵽�˰γ������
volatile bit flag_tim_scan_maybe_in_charging = 0; // ���ڸ���ʱ��ɨ��ı�־λ�����ܼ�⵽�˳����
volatile bit flag_tim_set_is_in_charging = 0;     // �ɶ�ʱ����λ/��λ�ģ���ʾ�Ƿ��в��������ı�־λ
volatile bit flag_tim_scan_bat_maybe_full = 0;    // ���ڸ���ʱ��ɨ��ı�־λ�����ܼ�⵽��ر�������
volatile bit flag_tim_set_bat_is_full = 0;        // �ɶ�ʱ����λ/��λ�ģ���ʾ����Ƿ񱻳�����ı�־λ

volatile bit flag_tim_scan_maybe_low_bat = 0; // ���ڸ���ʱ��ɨ��ı�־λ�����ܼ�⵽�˵͵���
volatile bit flag_tim_set_bat_is_low = 0;     // �ɶ�ʱ����λ/��λ�ģ���ʾ�ڹ���ʱ������Ƿ��ڵ͵����ı�־λ

volatile bit flag_tim_scan_maybe_shut_down = 0; // ���ڸ���ʱ��ɨ��ı�־λ�����ܼ�⵽�� ��ص�ѹ ���� �ػ���Ӧ�ĵ�ѹ
volatile bit flag_tim_set_shut_down = 0;        // �ɶ�ʱ����λ/��λ�ģ���ʾ�ڹ���ʱ����⵽�� ��ص�ѹ ��һ��ʱ���ڶ����� �ػ���Ӧ�ĵ�ѹ

volatile bit flag_tim_scan_maybe_motor_stalling = 0; // ���ڸ���ʱ��ɨ��ı�־λ�����ܼ�⵽�˵����ת
volatile bit flag_tim_set_motor_stalling = 0;        // �ɶ�ʱ����λ/��λ�ģ���ʾ�ڹ���ʱ��⵽�˵����ת

volatile bit flag_is_enable_key_scan = 0; // ��־λ���Ƿ�ʹ�ܰ���ɨ��(ÿ10ms����ʱ����λ�����������м�Ⲣ����)
volatile bit flag_is_dev_open = 0;        // �豸�Ƿ񿪻��ı�־λ��0--δ������1--����

volatile bit flag_ctl_led_blink = 0; // ���Ʊ�־λ���Ƿ����ָʾ����˸
volatile bit flag_ctl_turn_dir = 0;  // ���Ʊ�־λ���Ƿ��������ķ���

volatile bit flag_ctl_dev_close = 0; // ���Ʊ�־λ���Ƿ�Ҫ�ر��豸

volatile bit flag_ctl_low_bat_alarm = 0; // ���Ʊ�־λ���Ƿ�ʹ�ܵ͵�������

volatile bit flag_is_disable_to_open = 0; // ��־λ���Ƿ�ʹ�ܿ���(�͵�����������)

// ���ƺ���������
void fun_ctl_power_on(void)
{
    alter_motor_speed(2); // �����󣬵���������

    motor_pwm_b_enable(); // �������ת��

    cur_motor_dir = 1;    // ��ʾ��ǰ�������ת
    cur_motor_status = 2; // ���µ����λ״̬
    flag_is_dev_open = 1; // ��ʾ�豸�Ѿ�����

    // ���̵���˸
    LED_GREEN_ON();
    cur_sel_led = CUR_SEL_LED_GREEN;
    cur_ctl_led_blink_cnt = cur_motor_status; // led��˸�����뵱ǰ����ĵ�λ�й�
    flag_ctl_led_blink = 1;                   // ��LED��˸�Ĺ���
}

// ���ƺ������ػ�
void fun_ctl_power_off(void)
{
    // alter_motor_speed(0); �ػ���������Բ�д��һ����

    // �رյƹ���˸�Ķ���
    flag_ctl_led_blink = 0;
    LED_GREEN_OFF();
    LED_RED_OFF();

    // �رռ���
    cur_ctl_heat_status = 0;

    // �ر�PWM�����
    motor_pwm_disable();

    cur_motor_status = 0; // ��ʾ����Ѿ��ر�
    cur_motor_dir = 0;    // ���㣬�ص���ʼ״̬
    flag_is_dev_open = 0; // ��ʾ�豸�Ѿ��ر�

    // �ػ������� ���ʱ�����˹ػ����͵��������˹ػ����ֶ��ػ����Զ������˹ػ�
    // ����� ���ʱ�����˹ػ�����Ӧ�������Ӧ�ı�־λ
    if (0 == flag_is_in_charging)
    {
        // ���û���ڳ�磬��ո��͵����������޹صı�־λ
        flag_bat_is_full = 0;
        flag_tim_scan_maybe_not_charge = 0;
        flag_tim_scan_maybe_in_charging = 0;
        flag_tim_scan_bat_maybe_full = 0;
        flag_tim_scan_maybe_low_bat = 0;
        flag_tim_scan_maybe_shut_down = 0;
        flag_is_enable_key_scan = 0;
        flag_ctl_low_bat_alarm = 0;
    }
}

/**
 * @brief ���Ƶ����λ
 *           ���Ҫ���ݴ��������ڵ�λ����ı� ȫ�ֱ��� cur_motor_status ��״̬
 *
 * @param adjust_motor_status Ҫ���ڵ��ĵ�λ
 *                          0--����ȫ�ֱ��� cur_motor_status ��״̬���Զ�����
 *                          1--����Ϊ1��
 *                          2--����Ϊ2��
 *                          3--����Ϊ3��
 *                          ����--����ֱ�ӷ��� ��Ϊ�˽�ʡ����ռ䣬����û�мӣ�
 */
void fun_ctl_motor_status(u8 adjust_motor_status)
{
#if 0  // ���Խ�ʡ5�ֽڿռ�
    // if (adjust_motor_status > 3)
    // {
    //     return; // ��������ֱ�ӷ���
    // }
#endif // ���Խ�ʡ5�ֽڿռ�

    if (0 == adjust_motor_status)
    {
        // ������Ǹ��ݴ��������ڵ�λ��
        // ���Ǹ���ȫ�ֱ���cur_motor_status��״̬������
        if (1 == cur_motor_status)
        {
            // �� 1�� -> 2��
            cur_motor_status = 2;
        }
        else if (2 == cur_motor_status)
        {
            // �� 2�� -> 3��
            cur_motor_status = 3;
        }
        else if (3 == cur_motor_status)
        {
            // �� 3�� -> 1��
            cur_motor_status = 1;
        }
    }
    else
    {
        // ����Ǹ��ݴ��������ڵ�λ
        cur_motor_status = adjust_motor_status;
    }

    alter_motor_speed(cur_motor_status);

    // ÿ���л���λ�����ö�ʱ�����ض���
    flag_ctl_led_blink = 0; // ��ϵ�ǰ������˸�Ĺ���

    // ��������ڵ͵�������״̬����ʹ��LED��˸���ܣ�
    if (0 == flag_tim_set_bat_is_low)
    {
        if (0 == cur_ctl_heat_status)
        {
            // ���û�д򿪼��ȣ����̵���˸
            LED_RED_OFF();
            LED_GREEN_ON();
            cur_sel_led = CUR_SEL_LED_GREEN;
        }
        else
        {
            // ������˼��ȣ��ú����˸
            LED_GREEN_OFF();
            LED_RED_ON();
            cur_sel_led = CUR_SEL_LED_RED;
        }
        cur_ctl_led_blink_cnt = cur_motor_status; // led��˸�����뵱ǰ����ĵ�λ�й�
        flag_ctl_led_blink = 1;                   // ��LED��˸�Ĺ���
    }
}

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
void main(void)
{
    system_init();

    // �ر�HCK��HDA�ĵ��Թ���
    WDT_KEY = 0x55; // ���д����
    IO_MAP |= 0x03; // �ر�HCK��HDA���ŵĵ��Թ��ܣ����ӳ�䣩
    WDT_KEY = 0xBB;

    // ��ʼ����ӡ
#if USE_MY_DEBUG
    debug_init();
    printf("TXM8C101x_SDK main start\n");
#endif //  #if USE_MY_DEBUG

    // ��ʼ�� ��ⰴ�� �����ţ�
    // ������ P11 AIN9 �����ad���� ��������֮����ֱ�Ӽ���ƽ��
    P1_MD0 |= 0x03 << 2;   // ģ��ģʽ
    P1_AIOEN |= 0x01 << 1; // ʹ��ģ�⹦��

    // ʹ�� P00 ��Ϊ DEBUG ����
    P0_MD0 |= 0x01; // ���ģʽ

    heating_pin_config(); // ���ȿ�������
    led_config();
    tmr1_config();     // 1ms��ʱ��
    tmr2_pwm_config(); // ������ѹ��PWM

    adc_config();
    motor_config(); // ���Ƶ������·PWM

    // ����оƬ����֮���û�з�Ӧ����������ɫ����Ϊָʾ��
    LED_GREEN_ON();
    delay_ms(1000);
    LED_GREEN_OFF();

#if 0 // �ϵ�ʱ������Ƿ���ȷ��װ(����ͨ��):
    /*
        �����PWM�󣬼���صĵ�ѹ�����绹Ҫ�ߣ�˵��û�н����أ�
        ��⵽���ǳ��5V��ѹ��ĵ�ѹ
    */
    tmr2_pwm_enable();    // �򿪿�����ѹ��·��pwm
    TMR2_PWML = 93 % 256; // ����ΪԼ47.6%��ռ�ձ�
    TMR2_PWMH = 93 / 256;
    adc_sel_channel(ADC_CHANNEL_BAT); // �л�������ؽ�ѹ��ĵ�ѹ�ļ������

    {
        u8 i = 0;
        u16 adc_val = 0;
        for (i = 0; i < 10; i++) // ÿ55ms����һ�Σ�ѭ����ÿ�μ��Լ4.8ms
        {
            adc_val = adc_get_val();
            if (adc_val >= ADCDETECT_BAT_FULL + ADCDETECT_BAT_NULL_EX)
            {
                flag_bat_is_empty = 1; // ��ʾ����ǿյ�(���û�а�װ)
            }
        }
    }

    tmr2_pwm_disable(); // �رտ�����ѹ��·��PWM
    TMR2_PWML = 0;      // 0%ռ�ձ�
    TMR2_PWMH = 0;

#endif // �ϵ�ʱ������Ƿ���ȷ��װ
  
    while (1)
    {   

#if 0  // (����ͨ��)�ϵ�ʱ�������⵽���û�а�װ����LED��˸��ֱ�������ϵ�
        if (flag_bat_is_empty)
        {
            // û�з����أ�����LED��˸��ֱ�������ϵ�
            LED_RED_ON();
            delay_ms(200);
            LED_RED_OFF();
            delay_ms(200);
            continue;
        }
#endif // �ϵ�ʱ�������⵽���û�а�װ����LED��˸��ֱ�������ϵ�

#if 1
        charge_scan_handle();

        if (0 == flag_is_in_charging &&   /* �����ʱ���Ŷ԰��������ʹ��� */
            0 == flag_is_disable_to_open) /* �����ڵ͵������ܿ�����״̬ʱ */
        {
            if (flag_is_enable_key_scan) // ÿ10ms���ñ�־λ�ᱻ��ʱ����λһ��
            {
                flag_is_enable_key_scan = 0;
                key_scan_10ms_isr();
            }

            key_event_handle();
        }
        else
        {
            // flag_is_enable_key_scan = 0; // ֱ����ձ�־λ
        }
#endif

#if 1  // ����Զ�ת��
        if (flag_ctl_turn_dir)
        {
            // ���Ҫ�л����ת��
            flag_ctl_turn_dir = 0;
            if (0 == cur_motor_dir)
            {
                // �����ǰ���Ϊ��ʼ״̬��������
            }
            else if (1 == cur_motor_dir)
            {
                // �����ǰ���Ϊ��ת������Ϊ��ת
                // PWM0EC = 0; // �ر���ת��PWM���
                motor_pwm_b_disable(); // �ر���ת��PWM���
                delay_ms(500);
                // PWM1EC = 1;        // �򿪷�ת��PWM���
                motor_pwm_a_enable(); // �򿪷�ת��PWM���
                cur_motor_dir = 2;    // ��ʾ�����ǰת��Ϊ ��ת
            }
            else if (2 == cur_motor_dir)
            {
                // �����ǰ���Ϊ��ת������Ϊ��ת
                // PWM1EC = 0; // �رշ�ת��PWM���
                motor_pwm_a_disable(); // �رշ�ת��PWM���
                delay_ms(500);
                // PWM0EC = 1;        // ����ת��PWM���
                motor_pwm_b_enable();
                cur_motor_dir = 1; // ��ʾ�����ǰת��Ϊ ��ת
            }
            else
            {
                // ��������������ǣ���������
            }
        }
#endif // ����Զ�ת��

#if 1  // ����������ʹ���

        motor_over_current_detect_handle(); // �����ڲ���������û������
#endif // ����������ʹ���

#if 1  // ���ݿ��Ʊ�־λ�����ƹػ�
        if (flag_ctl_dev_close)
        {
            // ���Ҫ�ر��豸
            flag_ctl_dev_close = 0;
            fun_ctl_power_off();
        }
#endif // ���ݿ��Ʊ�־λ�����ƹػ�

#if 0 // ����豸״̬��ȷ���豸�Ƿ��Ѿ���������������Ӧ��־λ
    // ��������Բ��üӣ��Զ��ػ���������Ѿ������� ������ͼ��ȵ�״̬��

        // if ((0 != cur_motor_status) || /* ������δ�ر� */
        //     (0 != cur_ctl_heat_status) /* �������δ�ر� */
        // )
        // {
        //     flag_is_dev_open = 1; // ��ʾ�豸����
        // }

#endif

    } // while (1)
}

void TMR0_IRQHandler(void) interrupt TMR0_IRQn
{
    if (TMR0_CONH & 0x80)
    {
        TMR0_CONH |= 0x80;
        // 1ms����һ���жϣ����뵽����

        if (flag_bat_is_empty)
        {
            return; // ������Ϊ�գ���ǰ�˳��жϷ�����
        }

        { // ����ɨ��
            static volatile u8 key_scan_cnt = 0;
            key_scan_cnt++;
            if (key_scan_cnt >= 10)
            {
                key_scan_cnt = 0;
                flag_is_enable_key_scan = 1;
            }
        } // ����ɨ��

#if 1 // ����Ƿ�γ�/��������

        { // ����Ƿ�γ�/��������
            static u8 not_charge_ms_cnt = 0;
            static u8 charging_ms_cnt = 0;
            if (flag_is_in_charging)
            {
                if (flag_tim_scan_maybe_not_charge)
                {
                    // ���ڳ�磬���Ҽ�⵽�����г�����Ͽ������м�ʱ
                    not_charge_ms_cnt++;
                    if (not_charge_ms_cnt >= 50)
                    {
                        not_charge_ms_cnt = 0;
                        flag_tim_set_is_in_charging = 0;
                    }
                }
                else
                {
                    // ���ڳ�磬����û�м�⵽������Ͽ���
                    not_charge_ms_cnt = 0;
                    flag_tim_set_is_in_charging = 1;
                }
            }
            else
            {
                if (flag_tim_scan_maybe_in_charging)
                {
                    // û���ڳ�磬���Ǽ�⵽�г�������룬���м�ʱ��ȷ�ϳ�����Ƿ���Ĳ��룺
                    charging_ms_cnt++;
                    if (charging_ms_cnt >= 50)
                    {
                        charging_ms_cnt = 0;
                        flag_tim_set_is_in_charging = 1;
                    }
                }
                else
                {
                    // û���ڳ�磬����Ҳû�м�⵽���������
                    charging_ms_cnt = 0;
                    flag_tim_set_is_in_charging = 0;
                }
            }
        } // ����Ƿ�γ�/��������
#endif // ����Ƿ�γ�/��������

#if 1 // ���Ƶƹ���˸��Ч��

        { // ���Ƶƹ���˸��Ч��

            static volatile u16 blink_ms_cnt = 0; // ����ֵ�����Ƶƹ���˸��ʱ����

            // ״̬����
            // 0--��ʼֵ����״̬��
            // 1--׼��������˸��
            // 2--������˸
            static volatile u8 cur_blink_status = 0;
            static u8 __blink_cnt = 0; // һ��ʼ���Ҫ��˸�Ĵ�����������Ƶ�ǰʣ����˸����

            if (flag_ctl_led_blink)
            {
                // ���ʹ���˵ƹ���˸
                if (0 == cur_blink_status)
                {
                    // �����ǰ�ƹ�״̬���ڳ�ʼֵ��û������˸
                    cur_blink_status = 1;
                }
            }
            else
            {
                // �����ʹ�ܵƹ���˸������ֹͣ��ǰ�ƹ���˸��Ч��
                cur_blink_status = 0;
                blink_ms_cnt = 0;
                __blink_cnt = 0;
            }

            if (1 == cur_blink_status)
            {
                // ׼��������˸���ж�Ҫ��˸���Ǻ�ƻ����̵ƣ���Ҫ��ʾ��ǰ����ĵ�λ���Ǽ��ȵĵ�λ

                // ��������˸ǰ�ȹرյƹ⣬�ٵ���Ҫ��˸��LED��
                // �����ʹ����˸��Ч���������Ͳ��ü���������жϣ�
                // if (CUR_SEL_LED_RED == cur_sel_led)
                // {
                //     // ���Ҫ�ú����˸
                //     // LED_GREEN_OFF();
                //     // LED_RED_ON();
                // }
                // else if (CUR_SEL_LED_GREEN == cur_sel_led)
                // {
                //     // ���Ҫ���̵���˸
                //     // LED_RED_OFF();
                //     // LED_GREEN_ON();
                // }

                // �õƹⱣ�ֵ���500ms
                blink_ms_cnt++;
                if (blink_ms_cnt >= 500)
                {
                    blink_ms_cnt = 0;
                    cur_blink_status = 2; // ��ʾ��ʽ����LED��˸״̬
                }
            }
            else if (2 == cur_blink_status)
            {
                // ������ڽ���LED��˸
                if (0 == __blink_cnt)
                {
                    __blink_cnt = cur_ctl_led_blink_cnt; // ��ŵ�ǰҪ��˸�Ĵ���
                }

                if (__blink_cnt) // �����˸������Ϊ0��������˸
                {
                    blink_ms_cnt++;
                    if (blink_ms_cnt <= 500)
                    {
                        // Ϩ��LED 500ms
                        if (CUR_SEL_LED_RED == cur_sel_led)
                        {
                            // ���Ҫ�ú����˸
                            LED_RED_OFF();
                        }
                        else if (CUR_SEL_LED_GREEN == cur_sel_led)
                        {
                            // ���Ҫ���̵���˸
                            LED_GREEN_OFF();
                        }
                    }
                    else if (blink_ms_cnt < 1000)
                    {
                        // ����LED 500ms
                        if (CUR_SEL_LED_RED == cur_sel_led)
                        {
                            // ���Ҫ�ú����˸
                            LED_RED_ON();
                        }
                        else if (CUR_SEL_LED_GREEN == cur_sel_led)
                        {
                            // ���Ҫ���̵���˸
                            LED_GREEN_ON();
                        }
                    }
                    else
                    {
                        blink_ms_cnt = 0;
                        __blink_cnt--; // �����һ����˸������ֵ��һ
                        if (0 == __blink_cnt)
                        {
                            // ���ʣ����˸����Ϊ0��������˸
                            flag_ctl_led_blink = 0; // ��ʹ����˸
                            cur_blink_status = 0;   // �ָ�����ʼֵ����ʾû������˸
                        }
                    }
                }
            }
        } // ���Ƶƹ���˸��Ч��
#endif // ���Ƶƹ���˸��Ч��

#if 1 // ���Ƽ���

        { // ���Ƽ���
            static volatile u8 __ctl_heat_ms_cnt = 0;

            if (1 == cur_ctl_heat_status)
            {
                // һ������
                HEATING_ON();
            }
            else if (2 == cur_ctl_heat_status)
            {
                // ��������
                __ctl_heat_ms_cnt++;
                if (__ctl_heat_ms_cnt <= 5)
                {
                    HEATING_ON();
                }
                else if (__ctl_heat_ms_cnt < 10)
                {
                    HEATING_OFF();
                }
                else
                {
                    __ctl_heat_ms_cnt = 0;
                }
            }
            else
            {
                // ��״̬���رռ���
                HEATING_OFF();
                __ctl_heat_ms_cnt = 0;
            }

        } // ���Ƽ���
#endif // ���Ƽ���

#if 1 // �����Զ��ػ�

        { // �Զ��ػ�
            static volatile u32 shut_down_ms_cnt = 0;

            if ((0 != cur_motor_status ||     /* ���������ǹرյ� */
                 0 != cur_ctl_heat_status) && /* ������Ȳ��ǹرյ� */
                0 == flag_is_in_charging      /* ��ǰû���ڳ�� */
            )
            {
                shut_down_ms_cnt++;
                if (shut_down_ms_cnt >= 600000) // xx ms �󣬹ػ�
                {
                    shut_down_ms_cnt = 0;
                    flag_ctl_dev_close = 1;
                }
            }
            else
            {
                shut_down_ms_cnt = 0;
            }
        } // �Զ��ػ�
#endif // �����Զ��ػ�

#if 1     // ���Ƶ���Զ�������
        { // �Զ�������

            static volatile u16 turn_dir_ms_cnt = 0; // ����������ʱÿ xx min�л�һ��ת��ļ�ʱ����

            if ((0 != cur_motor_status) && /* ���������ǹرյ� */
                1                          /* ���û��ͨ���������ڹ����� */
            )
            {
                turn_dir_ms_cnt++;
                if (turn_dir_ms_cnt >= 60000) // xx ms �󣬻�����
                // if (turn_dir_ms_cnt >= 60) // xx ms �󣬻����򣨲����ã�
                {
                    turn_dir_ms_cnt = 0;
                    // ��־λ��һ���û�����Ĳ�����������ִ�У���Ϊ������Ҫ���500ms
                    flag_ctl_turn_dir = 1;
                }
            }
            else
            {
                turn_dir_ms_cnt = 0;
            }

        } // �Զ�������
#endif // ���Ƶ���Զ�������

#if 1 // ����Ƿ������

        { // ����Ƿ������
            static volatile u16 bat_is_full_ms_cnt = 0;
            if (flag_is_in_charging)
            {
                if (flag_tim_scan_bat_maybe_full)
                {
                    // ���ڳ�磬�����м�⵽������磬�����ۼƣ�
                    bat_is_full_ms_cnt++;
                    if (bat_is_full_ms_cnt >= 5000) // xx ms
                    {
                        bat_is_full_ms_cnt = 0;
                        flag_tim_set_bat_is_full = 1;
                    }
                }
                else
                {
                    // ���ڳ�磬����û�м�⵽������磺
                    bat_is_full_ms_cnt = 0;
                    flag_tim_set_bat_is_full = 0;
                }
            }
        } // ����Ƿ������
#endif // ����Ƿ������

#if 1 // ����Ƿ�Ҫ�͵�������

        {
            static u16 low_bat_ms_cnt = 0;
            if (flag_tim_scan_maybe_low_bat)
            {
                // ������ܼ�⵽�˵͵���������������ʱ
                low_bat_ms_cnt++;
                if (low_bat_ms_cnt >= LOW_BAT_SCAN_TIMES_MS)
                {
                    low_bat_ms_cnt = 0;
                    flag_tim_set_bat_is_low = 1;
                }
            }
            else
            {
                // ���û�м�⵽�͵���
                low_bat_ms_cnt = 0;
                flag_tim_set_bat_is_low = 0;
            }
        }

        { // ���ݱ�־λ��ִ�е͵��������Ĺ��ܣ�ִ��ǰ(�����Ʊ�־λ��һǰ)Ҫ�ȹر�����LED

            static bit __flag_is_in_low_bat_alarm = 0;   // ��־λ���Ƿ�����ִ�е͵��������Ĺ���
            static u16 __blink_cnt_in_low_bat_alarm = 0; // �͵�������ʱ��LED��˸ʱ�����

            /* ���ʹ���˵͵��������������豸�������� */
            if (flag_ctl_low_bat_alarm &&
                (0 != cur_motor_status || 0 != cur_ctl_heat_status))
            {
                if (0 == __flag_is_in_low_bat_alarm)
                {
                    __flag_is_in_low_bat_alarm = 1; // �ڸ������ڲ���ʹ�ܵ͵��������Ĺ���
                }
            }
            else
            {
                __blink_cnt_in_low_bat_alarm = 0;
                __flag_is_in_low_bat_alarm = 0;
            }

            if (__flag_is_in_low_bat_alarm)
            {
                __blink_cnt_in_low_bat_alarm++;
                if (__blink_cnt_in_low_bat_alarm <= 300)
                {
                    LED_RED_ON();
                }
                else if (__blink_cnt_in_low_bat_alarm < 600)
                {
                    LED_RED_OFF();
                }
                else
                {
                    __blink_cnt_in_low_bat_alarm = 0;
                }
            }
        } // ���ݱ�־λ��ִ�е͵��������Ĺ��ܣ�ִ��ǰ(�����Ʊ�־λ��һǰ)Ҫ�ȹر�����LED
#endif // ����Ƿ�Ҫ�͵�������

#if 1 // ����ʱ������ص����Ƿ�һֱ���ڹػ���ѹ

        {
            static u16 __shut_down_cnt = 0; // �Ե�ص�ѹ���ڹػ���ѹ��������ʱ

            if (flag_tim_scan_maybe_shut_down)
            {
                // �����⵽�ڹ���ʱ����ص�ѹ���ڹػ���ѹ������������ʱ
                __shut_down_cnt++;
                if (__shut_down_cnt >= SHUT_DOWN_SCAN_TIMES_MS)
                {
                    __shut_down_cnt = 0;
                    flag_tim_set_shut_down = 1;
                }
            }
            else
            {
                // ����ڹ���ʱ��û�м�⵽ ��ص�ѹ���ڹػ���ѹ
                __shut_down_cnt = 0;
                flag_tim_set_shut_down = 0;
            }
        }
#endif // ����ʱ������ص����Ƿ�һֱ���ڹػ���ѹ

#if 1 // ���������⣬����10s����Ϊ�����ת

        {
            // ��⵽�����ת�󣬽���������ʱ
            static u16 __detect_motor_stalling_cnt = 0;

            if (flag_tim_scan_maybe_motor_stalling && 0 != cur_motor_status)
            {
                // �����⵽�е����ת�����
                // P00 = 1;

                __detect_motor_stalling_cnt++;
                if (__detect_motor_stalling_cnt >= MOTOR_STALLING_SCAN_TIMES_MS)
                {
                    __detect_motor_stalling_cnt = 0;
                    flag_tim_set_motor_stalling = 1;
                }
            }
            else
            {
                // ���û�м�⵽�е����ת�����
                // P00 = 0;

                __detect_motor_stalling_cnt = 0;
                flag_tim_set_motor_stalling = 0;
            }
        }
#endif // ���������⣬����10s����Ϊ�����ת
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
