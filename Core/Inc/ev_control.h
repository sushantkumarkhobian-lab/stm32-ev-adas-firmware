/*
 * ev_control.h
 *
 *  Created on: Jun 15, 2026
 *      Author: Sushant Kumar
 */

#ifndef EV_CONTROL_H
#define EV_CONTROL_H

#include <stdint.h>

typedef enum
{
    DRIVE_ECO = 0,
    DRIVE_NORMAL,
    DRIVE_SPORT
} DriveMode_t;


typedef enum
{
    VEHICLE_PARKED = 0,
    VEHICLE_READY,
    VEHICLE_DRIVING,
    VEHICLE_REGEN,
    VEHICLE_FAULT
} VehicleState_t;

typedef struct
{
    float accelerator_percent;
    float brake_percent;

    float speed_kmph;
    float torque_nm;
    float motor_power_kw;
    float soc_percent;
    float estimated_range_km;

    VehicleState_t state;
    float temperature_c;

    DriveMode_t mode;
} EV_Data_t;

void EV_Init(EV_Data_t *ev);
void EV_Update(EV_Data_t *ev, float dt_seconds);

#endif
