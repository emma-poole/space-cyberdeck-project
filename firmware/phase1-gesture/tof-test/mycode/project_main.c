#include "main.h"
#include "project_main.h"
#include "app_tof.h"
#include "SEGGER_RTT.h"
#include <stdio.h>
#include "custom_ranging_sensor.h"
#include "custom.h"

static RANGING_SENSOR_Result_t Result;

void nonblocking_read_distance(void) {
    int32_t status = CUSTOM_RANGING_SENSOR_GetDistance(CUSTOM_VL53L4CX, &Result);

    // Debug print to see what's failing
    if (status != BSP_ERROR_NONE) {
        SEGGER_RTT_printf(0, "ToF Error Status: %d\r\n", status);
    } else {
        uint8_t targets = Result.ZoneResult[0].NumberOfTargets;
        if (targets > 0) {
            uint32_t dist_mm = Result.ZoneResult[0].Distance[0];
            SEGGER_RTT_printf(0, "Distance: %u mm\r\n", dist_mm);
        } else {
            SEGGER_RTT_printf(0, "No targets detected (0 targets)\r\n");
        }
    }
}

void project_main(void) {

    // Initialise variables
    uint32_t last_led_tick = 0;
    uint32_t last_tof_tick = 0;

    // Start sensor ranging once
    int32_t start_status = CUSTOM_RANGING_SENSOR_Start(CUSTOM_VL53L4CX, RS_MODE_BLOCKING_CONTINUOUS);
    SEGGER_RTT_printf(0, "ToF Start Status: %d\r\n", start_status);

    while (1) {

        uint32_t now = HAL_GetTick();

        // Blinking LED
        if (now - last_led_tick >= 500) {
            last_led_tick = now;
            HAL_GPIO_TogglePin(LED_BLINK_GPIO_Port, LED_BLINK_Pin);
            SEGGER_RTT_printf(0, "success!!\r\n"); 
        }

        // TOF reading test
        if (now - last_tof_tick >= 100) {
            last_tof_tick = now;
            nonblocking_read_distance();
        }
    }
}