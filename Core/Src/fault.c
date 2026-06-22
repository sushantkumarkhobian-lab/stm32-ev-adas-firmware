/*
 * fault.c
 *
 *  Created on: Jun 15, 2026
 *      Author: Sushant Kumar
 */


#include "fault.h"

void Fault_Init(FaultManager_t *fm)
{
    fm->fault_flags = FAULT_NONE;
    fm->alarm_priority = ALARM_P0_NONE;
}

void Fault_Update(FaultManager_t *fm, float temperature_c, float soc_percent)
{
    fm->fault_flags = FAULT_NONE;

    if(temperature_c > 90.0f)
    {
        fm->fault_flags |= FAULT_OT;
    }

    if(soc_percent < 2.0f)
    {
        fm->fault_flags |= FAULT_SOC;
    }

    if(fm->fault_flags & (FAULT_OT | FAULT_SOC | FAULT_COL | FAULT_SEN | FAULT_COM))
    {
        fm->alarm_priority = ALARM_P1_CRITICAL;
    }
    else if(temperature_c > 75.0f)
    {
        fm->alarm_priority = ALARM_P2_WARNING;
    }
    else if(soc_percent < 20.0f)
    {
        fm->alarm_priority = ALARM_P3_ADVISORY;
    }
    else
    {
        fm->alarm_priority = ALARM_P0_NONE;
    }
}

uint8_t Fault_HasCritical(FaultManager_t *fm)
{
    return fm->alarm_priority == ALARM_P1_CRITICAL;
}
