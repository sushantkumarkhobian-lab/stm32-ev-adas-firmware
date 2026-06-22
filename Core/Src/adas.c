/*
 * adas.c
 *
 *  Created on: Jun 16, 2026
 *      Author: Sushant Kumar
 */


#include "adas.h"

static ADAS_Data_t adas;

static uint8_t warning_clear_count = 0;
static uint8_t critical_clear_count = 0;

void ADAS_Init(void)
{
    adas.front_dist_cm = 400;
    adas.left_dist_cm = 400;
    adas.right_dist_cm = 400;

    adas.speed_kmph = 0;
    adas.ttc_sec = 99;

    adas.collision_level = ADAS_SAFE;

    adas.blind_left = 0;
    adas.blind_right = 0;

    adas.parking_assist = 0;
    adas.critical_fault = 0;
}

void ADAS_Update(
        float front_cm,
        float left_cm,
        float right_cm,
        float speed_kmph)
{
    adas.front_dist_cm = front_cm;
    adas.left_dist_cm = left_cm;
    adas.right_dist_cm = right_cm;

    adas.speed_kmph = speed_kmph;

    float speed_mps = speed_kmph / 3.6f;
    float front_m = front_cm / 100.0f;

    if(speed_mps > 0.1f)
    {
        adas.ttc_sec = front_m / speed_mps;
    }
    else
    {
        adas.ttc_sec = 99;
    }

    uint8_t warning_now = 0;
    uint8_t critical_now = 0;

    if(front_cm < 20 || adas.ttc_sec < 1.5f)
    {
        critical_now = 1;
    }
    else if(front_cm < 50 || adas.ttc_sec < 3.0f)
    {
        warning_now = 1;
    }

    if(critical_now)
    {
        adas.collision_level = ADAS_CRITICAL;
        adas.critical_fault = 1;
        critical_clear_count = 0;
    }
    else if(warning_now)
    {
        adas.collision_level = ADAS_WARNING;
        warning_clear_count = 0;
    }
    else
    {
        warning_clear_count++;

        if(warning_clear_count >= 3)
        {
            adas.collision_level = ADAS_SAFE;
            adas.critical_fault = 0;

            warning_clear_count = 0;
        }
    }

    if(speed_kmph > 20 && left_cm < 30)
    {
        adas.blind_left = 1;
    }
    else
    {
        adas.blind_left = 0;
    }

    if(speed_kmph > 20 && right_cm < 30)
    {
        adas.blind_right = 1;
    }
    else
    {
        adas.blind_right = 0;
    }

    if(speed_kmph < 10)
    {
        adas.parking_assist = 1;
    }
    else
    {
        adas.parking_assist = 0;
    }
}

ADAS_Data_t ADAS_GetData(void)
{
    return adas;
}
