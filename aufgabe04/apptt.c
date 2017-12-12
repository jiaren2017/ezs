#include <sml/ttkernel/c_api.h>
#include <pkgconf/ttkernel.h>
#include <stdio.h>
#include <inttypes.h>
//#include <cyg/kernel/kapi.h>

#include "ezs_io_fel.h"
#include "ezs_counter.h"
#include "ezs_trace.h"
#include "ezs_stopwatch.h"

//A little helper function
tt_ticktype ms_to_cyg_ticks(cyg_uint32 ms)
{
	cyg_uint64 newMs = ms;
	cyg_resolution_t resolution = ttEcos_get_resolution();
    newMs *= 1e6;
    newMs = newMs * resolution.divisor;
    newMs = newMs / resolution.dividend;
    //newMs = newMs * 1e3;
    //newMs = newMs / 1e9;
	return newMs;
}

tt_ticktype ms_to_ezs_ticks(cyg_uint32 ms)
{
	ms *= 1e6;
    ms = ms / ezs_counter_get_resolution().dividend;
    ms = ms * ezs_counter_get_resolution().divisor;
	return ms;
}

// Declare Dispatcher table
tt_DispatcherTable(table1, 100);

// Declare task t1
tt_Task(t1)
{
	ezs_lose_time(ms_to_ezs_ticks(2), 100);
    //ezs_printf("WAS1");
    //tt_statustype a = tt_gettaskid(t1);
    //ezs_printf("%d\n", a);
}

tt_Task(t2)
{
	ezs_lose_time(ms_to_ezs_ticks(2), 100);
    //ezs_printf("WAS2");
}

tt_Task(t3)
{
	ezs_lose_time(ms_to_ezs_ticks(3), 100);
    //ezs_printf("WAS3");
}

tt_Task(t4)
{
	ezs_lose_time(ms_to_ezs_ticks(6), 100);
    //ezs_printf("WAS4");
}


// Idle task does nothing...
ttIdleTask
{
	while(1);
}


externC void cyg_user_start()
{
	// Initialize counter
	ezs_counter_init();
	
	// Initialize tasks with stringent deadline checking
	tt_InitTask(t1, TT_STRINGENT);
    tt_InitTask(t2, TT_STRINGENT);
    tt_InitTask(t3, TT_STRINGENT);
    tt_InitTask(t4, TT_STRINGENT);
    

	// Initialize dispatcher table
	tt_InitDispatcherTable(table1);
	// Don't forget to test if table is valid
	bool valid = 1;
	/**
	 * BEWARE! Neiter ms_to_ezs_ticks nor ms_to_cyg_ticks are working!
	 * Fix them in order to complete this exercise
	 */
	// Insert entries:
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(0) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(9) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(2) , TT_START_TASK, t2 );
	if (!valid) ezs_printf("Task2");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(21) , TT_DEADLINE  , t2 );
	if (!valid) ezs_printf("Deadline2");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(4) , TT_START_TASK, t3 );
	if (!valid) ezs_printf("Task3");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(23) , TT_DEADLINE  , t3 );
	if (!valid) ezs_printf("Deadline3");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(10) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(19) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
     valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(12) , TT_START_TASK, t4 );
	if (!valid) ezs_printf("Task4");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(11) , TT_DEADLINE  , t4 );
	if (!valid) ezs_printf("Deadline4");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(20) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(29) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(22) , TT_START_TASK, t2 );
	if (!valid) ezs_printf("Task2");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(41) , TT_DEADLINE  , t2 );
	if (!valid) ezs_printf("Deadline2");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(24) , TT_START_TASK, t3 );
	if (!valid) ezs_printf("Task3");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(43) , TT_DEADLINE  , t3 );
	if (!valid) ezs_printf("Deadline3");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(30) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(39) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(40) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(49) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(42) , TT_START_TASK, t2 );
	if (!valid) ezs_printf("Task2");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(61) , TT_DEADLINE  , t2 );
	if (!valid) ezs_printf("Deadline2");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(44) , TT_START_TASK, t3 );
	if (!valid) ezs_printf("Task3");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(63) , TT_DEADLINE  , t3 );
	if (!valid) ezs_printf("Deadline3");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(50) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(59) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(60) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(69) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(62) , TT_START_TASK, t2 );
	if (!valid) ezs_printf("Task2");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(81) , TT_DEADLINE  , t2 );
	if (!valid) ezs_printf("Deadline2");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(64) , TT_START_TASK, t3 );
	if (!valid) ezs_printf("Task3");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(83) , TT_DEADLINE  , t3 );
	if (!valid) ezs_printf("Deadline3");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(70) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(79) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(80) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(89) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(82) , TT_START_TASK, t2 );
	if (!valid) ezs_printf("Task2");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(1) , TT_DEADLINE  , t2 );
	if (!valid) ezs_printf("Deadline2");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(84) , TT_START_TASK, t3 );
	if (!valid) ezs_printf("Task3");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(3) , TT_DEADLINE  , t3 );
	if (!valid) ezs_printf("Deadline3");
    
    valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(90) , TT_START_TASK, t1 );
	if (!valid) ezs_printf("Task1");
	valid &= tt_DispatcherTableEntry( table1, ms_to_cyg_ticks(99) , TT_DEADLINE  , t1 );
	if (!valid) ezs_printf("Deadline1");
    

	if(!valid)
	{
		ezs_printf("WARNING: Table not valid!\n");
	}
	else
	{
		ttStartOS(table1);
	}
}
