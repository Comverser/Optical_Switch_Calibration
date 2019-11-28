#include "cntrl_od.h"
#include "uart_pi.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h> // usleep()

struct param_area_sweep param_area =
{
    .fast.ch = 0,
    .fast.start_mv = 30000,
    .fast.end_mv = 0,
    .fast.step_mv = 10000,
    .fast.ut_step = 28000,

    .slow.ch = 1,
    .slow.start_mv = 0,
    .slow.end_mv = 30000,
    .slow.step_mv = 10000,
    .slow.ut_step = 28000
};

pthread_t t;

atomic_int curr_volt_f = 0;
atomic_int curr_volt_s = 0;

atomic_bool is_running = 0;

void control_init()
{
    init_uart();
}

void control_volt(int p_ch, int p_mv)
{
    char str1[8];
    char str2[6];

    if ( p_ch == 0)
    {
        strcpy(str1, "x ");
    }
    else if ( p_ch == 1 )
    {
        strcpy(str1, "y ");
    }
    else
    {
        fprintf(stderr, "UART error\n");
    }

    sprintf(str2, "%d", p_mv);

    strcat(str1, str2);

    if ( transmit_receive_uart((unsigned char *)str1) )
    {
        fprintf(stderr, "UART error at %d mV\n", p_mv);
    }
}

void sweep_volt(struct param_sweep ch_swp)
{
    // Check sweep direction
    if ( ch_swp.start_mv > ch_swp.end_mv )
    {
        ch_swp.step_mv= -ch_swp.step_mv;
    }

    curr_volt_f = ch_swp.start_mv;
    control_volt(ch_swp.ch, curr_volt_f);
    usleep(ch_swp.ut_step);

    while (is_running)
    {
        atomic_fetch_add(&curr_volt_f, ch_swp.step_mv);

        // Check sweep direction
        if ( ch_swp.start_mv < ch_swp.end_mv )
        {
            if ( curr_volt_f > ch_swp.end_mv )
            {
                atomic_fetch_sub(&curr_volt_f, ch_swp.step_mv);
                break;
            }
        }
        else
        {
            if ( curr_volt_f < ch_swp.end_mv )
            {
                atomic_fetch_sub(&curr_volt_f, ch_swp.step_mv);
                break;
            }
        }

        control_volt(ch_swp.ch, curr_volt_f);
        usleep(ch_swp.ut_step);
    }
}

void area_sweep_volt(struct param_area_sweep area_swp)
{
    // Check sweep direction
    if ( area_swp.slow.start_mv > area_swp.slow.end_mv )
    {
        area_swp.slow.step_mv= -area_swp.slow.step_mv;
    }

    curr_volt_s = area_swp.slow.start_mv;
    control_volt(area_swp.slow.ch, curr_volt_s);
    sweep_volt(area_swp.fast);

    while (is_running)
    {
        atomic_fetch_add(&curr_volt_s, area_swp.slow.step_mv);

        // Check sweep direction
        if ( area_swp.slow.start_mv < area_swp.slow.end_mv )
        {
            if ( curr_volt_s > area_swp.slow.end_mv )
            {
                atomic_fetch_sub(&curr_volt_s, area_swp.slow.step_mv);
                break;
            }
        }
        else
        {
            if ( curr_volt_s < area_swp.slow.end_mv )
            {
                atomic_fetch_sub(&curr_volt_s, area_swp.slow.step_mv);
                break;
            }
        }

        control_volt(area_swp.slow.ch, curr_volt_s);
        sweep_volt(area_swp.fast);
    }
}

void* thrd_sweep(void* ptr)
{
    sweep_volt( *(struct param_sweep*)ptr );
    return NULL;
}

void* thrd_area_sweep(void* ptr)
{
    area_sweep_volt( *(struct param_area_sweep*)ptr );
    return NULL;
}

