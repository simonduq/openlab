#include <string.h>
#include "platform.h"
#include "printf.h"
#include "scanf.h"
#include "soft_timer_delay.h"


static void test_task(void *param)
{
    int i = 0;
    int a;
    while (1) {
        printf("%d\n", i++);
        sscanf("1", "%d", &a);
        soft_timer_delay_ms(10);
    }
}

int main()
{
    platform_init();
    xTaskCreate(test_task, (const signed char *const) "test_task",
            configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    platform_run();
    return 0;
}
