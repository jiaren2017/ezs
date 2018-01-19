#include <cyg/hal/hal_arch.h>
#include <cyg/kernel/kapi.h>
#include <stdio.h>

#include "common.h"
#include "ezs_counter.h"
#include "ezs_io.h"
#include "ezs_stopwatch.h"

/*
 * Die einzelnen Teilaufgaben sollen in den Dateien
 *
 * aufgabe_1.c
 * aufgabe_2.c
 * aufgabe_3.c
 *
 * geloest werden. Diese sind fuer die Bearbeitung der Teilaufgabe jeweils in
 * CMakeLists.txt _einzeln_ einzukommentieren.
 *
 * Datenstrukturen, die sowohl in app.c als auch in den Implementierungen der
 * Teilaufgaben verwendet werden, sollen in common.h deklariert werden.
 *
 * Programmcode, der allen Teilaufgaben gemeinsam ist, soll wie gewohnt in
 * app.c implementiert werden.
 *
 */

static cyg_uint64 res_ps;

void cyg_user_start(void)
{
	ezs_counter_init();
	res_ps = ezs_counter_resolution_ps();

	cyg_clock_to_counter(cyg_real_time_clock(), &s_real_time_counter);
    	res = ezs_counter_get_resolution(); // get resolution ns/ticks

	init_tasks();
}

void lose_time_us(int us, int percentage)
{
	ezs_lose_time(us * 1000000LL / res_ps, percentage);
}
