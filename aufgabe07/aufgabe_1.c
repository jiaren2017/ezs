#include <cyg/kernel/kapi.h>
#include <stdio.h>

#include "ezs_stopwatch.h"
#include "ezs_io.h"

#include "common.h"


// in milliseconds

#define HIGH_TASK_PHASE   4
#define MEDIUM_TASK_PHASE 3
#define LOW_TASK_PHASE    1

#define HIGH_TASK_PERIOD   20
#define MEDIUM_TASK_PERIOD 50
#define LOW_TASK_PERIOD    200

#define HIGH_TASK_PRIORITY   10
#define MEDIUM_TASK_PRIORITY 11
#define LOW_TASK_PRIORITY    12


static cyg_uint32 time;
static cyg_uint32 ticks; 

static cyg_uint8     high_task_stack[STACKSIZE];
static cyg_thread    high_task_thread;

static cyg_uint8     medium_task_stack[STACKSIZE];
static cyg_thread    medium_task_thread;

static cyg_uint8     low_task_stack[STACKSIZE];
static cyg_thread    low_task_thread;

cyg_handle_t  high_task_handle;
cyg_handle_t  medium_task_handle;
cyg_handle_t  low_task_handle;

static void high_task_entry(cyg_addrword_t data);
static void medium_task_entry(cyg_addrword_t data);
static void low_task_entry(cyg_addrword_t data);

static cyg_tick_count_t ms_to_cyg_ticks(cyg_uint32 ms)
{
	
    cyg_uint64 newMs = ms;
	cyg_resolution_t resolution = cyg_clock_get_resolution(cyg_real_time_clock());
    //ezs_printf("ms: %d\n", ms);
    //ezs_printf("dividend: %d\n", resolution.dividend);
    //ezs_printf("divisor: %d\n", resolution.divisor);
    newMs *= 1e6;
    newMs = newMs * resolution.divisor;
    newMs = newMs / resolution.dividend;
    //ezs_printf("ICH WAR DA!! %d\n", newMs);
	return newMs;
}

static cyg_handle_t s_high_task_alarm_handle;
static cyg_alarm s_high_task_alarm;
static cyg_handle_t s_medium_task_alarm_handle;
static cyg_alarm s_medium_task_alarm;
static cyg_handle_t s_low_task_alarm_handle;
static cyg_alarm s_low_task_alarm;
/*
static cyg_handle_t s_low_lock_alarm_handle;
static cyg_alarm s_low_lock_alarm;
static cyg_handle_t s_high_lock_alarm_handle;
static cyg_alarm s_high_lock_alarm;
static cyg_handle_t s_low_unlock_alarm_handle;
static cyg_alarm s_low_unlock_alarm;
static cyg_handle_t s_high_unlock_alarm_handle;
static cyg_alarm s_high_unlock_alarm;
*/
static void high_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
        ezs_watch_start(&time);
		cyg_thread_resume(high_task_handle);
}

static void medium_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
		cyg_thread_resume(medium_task_handle);
}

static void low_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
		cyg_thread_resume(low_task_handle);
}
/*
static void low_task_lock(cyg_handle_t alarmH, cyg_addrword_t data)
{
        //ezs_printf("LOCK_LOW\n");
        cyg_scheduler_lock();
        cyg_alarm_disable(s_low_lock_alarm_handle);
}

static void high_task_lock(cyg_handle_t alarmH, cyg_addrword_t data)
{
        //ezs_printf("LOCK_HIGH\n");
        cyg_scheduler_lock();	
        cyg_alarm_disable(s_high_lock_alarm_handle);
}

static void low_task_unlock(cyg_handle_t alarmH, cyg_addrword_t data)
{
        //ezs_printf("UNLOCK_LOW\n");
        ticks = ezs_watch_stop(&time);
        cyg_scheduler_unlock();
        cyg_alarm_disable(s_low_unlock_alarm_handle);
    
}

static void high_task_unlock(cyg_handle_t alarmH, cyg_addrword_t data)
{
        //ezs_printf("UNLOCK_HIGH\n");    
        cyg_scheduler_unlock();	
        cyg_alarm_disable(s_high_unlock_alarm_handle);
}

*/


void init_tasks(void)
{
    cyg_uint32 timebase = cyg_current_time() + 3;
    
	cyg_thread_create(HIGH_TASK_PRIORITY, &high_task_entry, 0, "high priority task",
			high_task_stack, STACKSIZE,
			&high_task_handle, &high_task_thread);
	cyg_thread_create(MEDIUM_TASK_PRIORITY, &medium_task_entry, 0, "medium priority task",
			medium_task_stack, STACKSIZE,
			&medium_task_handle, &medium_task_thread);
	cyg_thread_create(LOW_TASK_PRIORITY, &low_task_entry, 0, "low priority task",
			low_task_stack, STACKSIZE,
			&low_task_handle, &low_task_thread);

	cyg_alarm_create(s_real_time_counter, high_task_alarmfn, data_dummy, &s_high_task_alarm_handle, &s_high_task_alarm);
	cyg_alarm_initialize(s_high_task_alarm_handle, timebase + ms_to_cyg_ticks(HIGH_TASK_PHASE), timebase + ms_to_cyg_ticks(HIGH_TASK_PERIOD));
	cyg_alarm_create(s_real_time_counter, medium_task_alarmfn, data_dummy, &s_medium_task_alarm_handle, &s_medium_task_alarm);
	cyg_alarm_initialize(s_medium_task_alarm_handle, timebase + ms_to_cyg_ticks(MEDIUM_TASK_PHASE), timebase + ms_to_cyg_ticks(MEDIUM_TASK_PERIOD));
	cyg_alarm_create(s_real_time_counter, low_task_alarmfn, data_dummy, &s_low_task_alarm_handle, &s_low_task_alarm);
	cyg_alarm_initialize(s_low_task_alarm_handle, timebase + ms_to_cyg_ticks(LOW_TASK_PHASE), timebase + ms_to_cyg_ticks(LOW_TASK_PERIOD));
    
    /*cyg_alarm_create(s_real_time_counter, low_task_lock, data_dummy, &s_low_lock_alarm_handle, &s_low_lock_alarm);
    cyg_alarm_initialize(s_low_lock_alarm_handle, ms_to_cyg_ticks(1), 0);
    cyg_alarm_disable(s_low_lock_alarm_handle);
    cyg_alarm_create(s_real_time_counter, high_task_lock, data_dummy, &s_high_lock_alarm_handle, &s_high_lock_alarm);
    cyg_alarm_initialize(s_high_lock_alarm_handle, ms_to_cyg_ticks(3), 0);
    cyg_alarm_disable(s_high_lock_alarm_handle);
    cyg_alarm_create(s_real_time_counter, low_task_unlock, data_dummy, &s_low_unlock_alarm_handle, &s_low_unlock_alarm);
    cyg_alarm_initialize(s_low_unlock_alarm_handle, ms_to_cyg_ticks(8), 0);
    cyg_alarm_disable(s_low_unlock_alarm_handle);
    cyg_alarm_create(s_real_time_counter, high_task_unlock, data_dummy, &s_high_unlock_alarm_handle, &s_high_unlock_alarm);
    cyg_alarm_initialize(s_high_unlock_alarm_handle, ms_to_cyg_ticks(4), 0);
    cyg_alarm_disable(s_high_unlock_alarm_handle);*/
    

}

static void high_task_entry(cyg_addrword_t data)
{
	while (1) {
        //ezs_printf("HIGH_TASK\n");
        //cyg_alarm_initialize(s_high_lock_alarm_handle, ms_to_cyg_ticks(3), 0);
        //cyg_alarm_initialize(s_high_unlock_alarm_handle, ms_to_cyg_ticks(4), 0);
        //cyg_alarm_enable(s_high_lock_alarm_handle);
        //cyg_alarm_enable(s_high_unlock_alarm_handle);
        lose_time_us(3000, 0);
        cyg_scheduler_lock();
        lose_time_us(1000, 0);
        cyg_scheduler_unlock();
        lose_time_us(2000, 0);
        //ezs_printf(ticks);
		cyg_thread_suspend(cyg_thread_self());

	}
}

static void medium_task_entry(cyg_addrword_t data)
{
	while (1) {
        //ezs_printf("MEDIUM_TASK\n");
        lose_time_us(4000, 100);
		cyg_thread_suspend(cyg_thread_self());

	}
}

static void low_task_entry(cyg_addrword_t data)
{
	while (1) {
        //ezs_printf("LOW_TASK\n");
        //cyg_alarm_initialize(s_low_lock_alarm_handle, ms_to_cyg_ticks(1), 0);
        //cyg_alarm_initialize(s_low_unlock_alarm_handle, ms_to_cyg_ticks(8), 0);
        //cyg_alarm_enable(s_low_lock_alarm_handle);
        //cyg_alarm_enable(s_low_unlock_alarm_handle);
        lose_time_us(1000, 0);
        cyg_scheduler_lock();
        lose_time_us(7000, 0);
        cyg_scheduler_unlock();
        lose_time_us(1000, 0);
		cyg_thread_suspend(cyg_thread_self());

	}
}

