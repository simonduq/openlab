#include "platform.h"
#include <stdlib.h> // atoi
#include "printf.h"

char *grace_time_ms = "0010";
/*
	to configure grace-time, use:
	
	sed -i 's/0010/xxxx/' ../../../bin/test_serial_flood.elf
*/

int main()
{
	platform_init();

	int k = 0;
	while(1)
	{
		printf("0x%x\n", ++k);
		int i;
		for (i=0; i < soft_timer_ms_to_ticks(atoi(grace_time_ms)); i++)
			;
	}
	return 0;
}
