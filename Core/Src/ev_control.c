/*
 * ev_control.c
 *
 *  Created on: Jun 15, 2026
 *      Author: Sushant Kumar
 */

#include "ev_control.h"

#define MAX_SPEED_KMPH        200.0f
#define MAX_TORQUE_NM         250.0f
#define MAX_REGEN_TORQUE_NM   -80.0f

#define VEHICLE_MASS_KG       1200.0f
#define WHEEL_RADIUS_M        0.30f
#define BATTERY_CAPACITY_KWH  40.0f

#define ECO_EFFICIENCY        0.13f
#define NORMAL_EFFICIENCY     0.16f
#define SPORT_EFFICIENCY      0.22f

static float clamp(float value, float min, float max)
{
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

void EV_Init(EV_Data_t *ev)
{
    ev->accelerator_percent = 0.0f;
    ev->brake_percent = 0.0f;

    ev->speed_kmph = 0.0f;
    ev->torque_nm = 0.0f;
    ev->motor_power_kw = 0.0f;
    ev->soc_percent = 100.0f;
    ev->estimated_range_km = 250.0f;

    ev->mode = DRIVE_NORMAL;

    ev->state = VEHICLE_PARKED;
    ev->temperature_c = 30.0f;
}

void EV_Update(EV_Data_t *ev, float dt_seconds)
{
    float mode_scale = 1.0f;
    float efficiency = NORMAL_EFFICIENCY;

    if(ev->mode == DRIVE_ECO)
    {
        mode_scale = 0.6f;
        efficiency = ECO_EFFICIENCY;
    }
    else if(ev->mode == DRIVE_NORMAL)
    {
        mode_scale = 1.0f;
        efficiency = NORMAL_EFFICIENCY;
    }
    else if(ev->mode == DRIVE_SPORT)
    {
        mode_scale = 1.3f;
        efficiency = SPORT_EFFICIENCY;
    }

    ev->accelerator_percent = clamp(ev->accelerator_percent, 0.0f, 100.0f);
    ev->brake_percent = clamp(ev->brake_percent, 0.0f, 100.0f);

    if(ev->brake_percent > 5.0f)
    {
        ev->torque_nm = MAX_REGEN_TORQUE_NM * (ev->brake_percent / 100.0f);

        float regen_power_kw = (-ev->torque_nm * ev->speed_kmph) / 9550.0f;
        ev->motor_power_kw = -regen_power_kw;

        ev->soc_percent += regen_power_kw * dt_seconds / 3600.0f / BATTERY_CAPACITY_KWH * 100.0f;

        ev->speed_kmph -= ev->brake_percent * 0.04f;
    }
    else
    {
        ev->torque_nm = MAX_TORQUE_NM * (ev->accelerator_percent / 100.0f) * mode_scale;

        float force = ev->torque_nm / WHEEL_RADIUS_M;
        float acceleration = force / VEHICLE_MASS_KG;

        ev->speed_kmph += acceleration * dt_seconds * 3.6f;

        ev->motor_power_kw = (ev->torque_nm * ev->speed_kmph) / 9550.0f;

        ev->soc_percent -= ev->motor_power_kw * dt_seconds / 3600.0f / BATTERY_CAPACITY_KWH * 100.0f;
    }

    ev->speed_kmph = clamp(ev->speed_kmph, 0.0f, MAX_SPEED_KMPH);
    ev->soc_percent = clamp(ev->soc_percent, 0.0f, 100.0f);

    ev->estimated_range_km =
        (ev->soc_percent / 100.0f) * BATTERY_CAPACITY_KWH / efficiency;
}
