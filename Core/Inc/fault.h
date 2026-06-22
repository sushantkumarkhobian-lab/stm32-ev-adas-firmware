/*
 * fault.h
 *
 *  Created on: Jun 15, 2026
 *      Author: Sushant Kumar
 */

#ifndef FAULT_H
#define FAULT_H

#include <stdint.h>

typedef enum
{
    ALARM_P0_NONE = 0,
    ALARM_P1_CRITICAL,
    ALARM_P2_WARNING,
    ALARM_P3_ADVISORY
} AlarmPriority_t;

#define FAULT_NONE  0x00
#define FAULT_OT    0x01
#define FAULT_SOC   0x02
#define FAULT_COL   0x04
#define FAULT_SEN   0x08
#define FAULT_COM   0x10

typedef struct
{
    uint8_t fault_flags;
    AlarmPriority_t alarm_priority;
} FaultManager_t;

void Fault_Init(FaultManager_t *fm);
void Fault_Update(FaultManager_t *fm, float temperature_c, float soc_percent);
uint8_t Fault_HasCritical(FaultManager_t *fm);

#endif
