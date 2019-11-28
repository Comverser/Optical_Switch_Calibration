#ifndef CNTRL_OD_H
#define CNTRL_OD_H

#include <stdatomic.h>
#include <pthread.h>

extern atomic_int curr_volt_f;
extern atomic_int curr_volt_s;

extern atomic_bool is_running;

extern pthread_t t;

struct param_sweep
{
    int ch;
    int start_mv;
    int end_mv;
    int step_mv;
    int ut_step; // Additional 22 ms delay exists
};

struct param_area_sweep
{
    struct param_sweep fast;
    struct param_sweep slow;
};

extern struct param_area_sweep param_area;

void control_init();
void control_volt(int p_ch, int p_mv);
void sweep_volt(struct param_sweep ch_swp);
void area_sweep_volt(struct param_area_sweep area_swp);

void* thrd_sweep(void* ptr);
void* thrd_area_sweep(void* ptr);

#endif
