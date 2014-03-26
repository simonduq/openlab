/*
 * Idle program
 *
 */

#include <stdint.h>
#include <math.h>
#include <string.h>

#include "platform.h"

int main()
{
	platform_init(); // Initialize the platform
	platform_run();  // Run
	return 0;
}
