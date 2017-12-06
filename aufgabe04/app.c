#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/kernel/kapi.h>
#include <stdio.h>

#include "ezs_counter.h"
#include "ezs_trace.h"
#include "ezs_io_fel.h"
#include "ezs_sensor.h"
#include "ezs_dac.h"
#include "ezs_stopwatch.h"
#include "ezs_serial.h"

#define STACKSIZE CYGNUM_HAL_STACK_SIZE_MINIMUM+1024


// Thread 1
static cyg_uint8 my_stack1[STACKSIZE];
static cyg_handle_t threadhndl1;
static cyg_thread   threaddata1;
static cyg_alarm    alarm1;
static cyg_handle_t alarmhnd1;

static cyg_uint8 my_stack2[STACKSIZE];
static cyg_handle_t threadhndl2;
static cyg_thread   threaddata2;
static cyg_alarm    alarm2;
static cyg_handle_t alarmhnd2;

static cyg_uint8 my_stack3[STACKSIZE];
static cyg_handle_t threadhndl3;
static cyg_thread   threaddata3;
static cyg_alarm    alarm3;
static cyg_handle_t alarmhnd3;

static cyg_uint8 my_stack4[STACKSIZE];
static cyg_handle_t threadhndl4;
static cyg_thread   threaddata4;
static cyg_alarm    alarm4;
static cyg_handle_t alarmhnd4;

static cyg_handle_t counter1;
static cyg_handle_t counter2;
static cyg_handle_t counter3;
static cyg_handle_t counter4;



void alarm_handler1(cyg_handle_t alarm, cyg_addrword_t data)
{
	// 'data' is a pointer to a thread handle! (see cyg_alarm_create below)
	cyg_thread_resume(*((cyg_handle_t*) data));
}

void alarm_handler2(cyg_handle_t alarm, cyg_addrword_t data)
{
	// 'data' is a pointer to a thread handle! (see cyg_alarm_create below)
	cyg_thread_resume(*((cyg_handle_t*) data));
}

void alarm_handler3(cyg_handle_t alarm, cyg_addrword_t data)
{
	// 'data' is a pointer to a thread handle! (see cyg_alarm_create below)
	cyg_thread_resume(*((cyg_handle_t*) data));
}

void alarm_handler4(cyg_handle_t alarm, cyg_addrword_t data)
{
	// 'data' is a pointer to a thread handle! (see cyg_alarm_create below)
	cyg_thread_resume(*((cyg_handle_t*) data));
}


//A little helper function.
cyg_tick_count_t ms_to_cyg_ticks(cyg_uint32 ms)
{
	 
	return ms;
}

//A little helper function.
cyg_tick_count_t ms_to_ezs_ticks(cyg_uint32 ms)
{
	ms *= 1e6;
    ms = ms / ezs_counter_get_resolution().divisor;
    ms = ms * ezs_counter_get_resolution().dividend;
	return ms;
}static cyg_handle_t counter;
/*volatile int x = CYG_FB_WIDTH(FRAMEBUF);

// A little test thread.
/*void thread(cyg_addrword_t arg)
{

	/**
	 * Example for the usage of the framebuffer.
	 * Comment out for simulating the SimpleScope!
	 * Reuse for displaying the pds!
	 **//*
	int i;
	int xstart = 14*16 /* chars */ /*+ 10 /* margin */;/*
	int sizex  = ( CYG_FB_WIDTH(FRAMEBUF) - xstart ) / 4;
	int sizey  = CYG_FB_HEIGHT(FRAMEBUF) /4;

	for( i = 0; i < 16; i++)
	{
		int col = sizex * (i % 4);
		int row = sizey * (i / 4);
my_stack
		ezs_fb_fill_block(xstart + col, row, sizex, sizey, i);

	}
	int size = 16;
	for(i=0; i < 94; i++)
	{
		int row = size * (i % 14);
		int col = size * (i / 14);

		ezs_fb_print_char(i+'!', row+size, col+size,  CYG_FB_DEFAULT_PALETTE_BLUE);
	}

	ezs_fb_print_string("Hallo Welt!",60,150, CYG_FB_DEFAULT_PALETTE_RED);

	while(1)alarmhnd1
	{
		/* Use this rather than printf, for EZS FrameBuffer Simulator 3000 *//*
		ezs_printf("Hallo!\n");
		cyg_thread_suspend(cyg_thread_self());
	}
	
	
	
	
}*/

void abtastung1(cyg_addrword_t arg){
    while(1){
        ezs_lose_time(ms_to_ezs_ticks(2), 100);
        cyg_thread_suspend(threadhndl1);
    }
}

void abtastung2(cyg_addrword_t arg){
    while(1){
        ezs_lose_time(ms_to_ezs_ticks(2), 100);
        cyg_thread_suspend(threadhndl2);
    }
}

void analyse(cyg_addrword_t arg){
    while(1){
        ezs_lose_time(ms_to_ezs_ticks(3), 100);
        cyg_thread_suspend(threadhndl3);
    }
}

void darstellung(cyg_addrword_t arg){
    while(1){
        ezs_lose_time(ms_to_ezs_ticks(4), 100);
        cyg_thread_suspend(threadhndl4);
    }
}



void cyg_user_start(void)
{
	ezs_serial_init();
	ezs_sensors_init();
	ezs_dac_init();

	// Initialize framebuffer in graphic mode
	ezs_fb_init();

	// Initialize EZS counter
	ezs_counter_init();

	// Create test thread
	cyg_thread_create(1, &abtastung1, 0, "Abtastung1", my_stack1, STACKSIZE,
	                  &threadhndl1, &threaddata1);
    
    cyg_thread_create(2, &abtastung2, 0, "Abtastung2", my_stack2, STACKSIZE,
	                  &threadhndl2, &threaddata2);
    
    cyg_thread_create(3, &analyse, 0, "Analyse", my_stack3, STACKSIZE,
	                  &threadhndl3, &threaddata3);
    
    cyg_thread_create(4, &darstellung, 0, "Darstellung", my_stack4, STACKSIZE,
	                  &threadhndl4, &threaddata4);

	cyg_clock_to_counter(cyg_real_time_clock(), &counter);

       /**
        * BEWARE! Neiter ms_to_ezs_ticks nor ms_to_cyg_ticks are working!
        * Fix them in order to complete this exercise
        */

	// Create alarm. Notice the pointer to the threadhndl1 as alarm function parameter!
	cyg_alarm_create(counter1, alarm_handler1, (cyg_addrword_t) &threadhndl1 , &alarmhnd1, &alarm1);
	cyg_alarm_initialize(alarmhnd1, cyg_current_time() + 1, ms_to_cyg_ticks(10));
	cyg_alarm_enable(alarmhnd1);
    
    cyg_alarm_create(counter2, alarm_handler2, (cyg_addrword_t) &threadhndl2 , &alarmhnd2, &alarm2);
	cyg_alarm_initialize(alarmhnd2, cyg_current_time() + 1, ms_to_cyg_ticks(20));
	cyg_alarm_enable(alarmhnd2);
    
    cyg_alarm_create(counter3, alarm_handler3, (cyg_addrword_t) &threadhndl3 , &alarmhnd3, &alarm3);
	cyg_alarm_initialize(alarmhnd3, cyg_current_time() + 1, ms_to_cyg_ticks(20));
	cyg_alarm_enable(alarmhnd3);
    
    cyg_alarm_create(counter4, alarm_handler4, (cyg_addrword_t) &threadhndl4 , &alarmhnd4, &alarm4);
	cyg_alarm_initialize(alarmhnd4, cyg_current_time() + 1, ms_to_cyg_ticks(100));
	cyg_alarm_enable(alarmhnd4);

}

