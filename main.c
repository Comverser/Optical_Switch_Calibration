//#include <stdlib.h>
#include "cntrl_od.h"

#include <stdio.h>

int main(int argc, char const *argv[])
{
    control_init();
    param_area.fast.ut_step = 1000000;
    param_area.slow.ut_step = 2000000;
    /*
    param_area.fast.ch = 0;
    param_area.fast.start_mv = 30000;
    param_area.fast.end_mv = 0;
    param_area.fast.step_mv = 3000;
    param_area.fast.ut_step = 3000+2*1000;

    param_area.slow.ch = 1;
    param_area.slow.start_mv = 0;
    param_area.slow.end_mv = 30000;
    param_area.slow.step_mv = 6000;
    param_area.slow.ut_step = 3000+2*1000+500000;
    */

    control_volt(0, 15000);
    control_volt(1, 15000);

    atomic_flag_test_and_set(&is_running);
    /*
    pthread_create(&t, NULL, thrd_sweep, &(param_area.fast));
    pthread_join(t, NULL);
    pthread_create(&t, NULL, thrd_sweep, &(param_area.slow));
    pthread_join(t, NULL);
    */
    printf("\ncurr_volt_f: %d \n", curr_volt_f);
    printf("\ncurr_volt_s: %d \n", curr_volt_s);
    pthread_create(&t, NULL, thrd_area_sweep, &param_area);
    printf("Enter value : ");
    getchar();
    atomic_flag_clear(&is_running);
//    atomic_fetch_sub(&is_running, 1);
    printf("\ncurr_volt_f: %d \n", curr_volt_f);
    printf("\ncurr_volt_s: %d \n", curr_volt_s);
    pthread_join(t, NULL);
    printf("\ncurr_volt_f: %d \n", curr_volt_f);
    printf("\ncurr_volt_s: %d \n", curr_volt_s);

//    control_volt(0, 15000);
//    control_volt(1, 15000);

    return 0;
}
