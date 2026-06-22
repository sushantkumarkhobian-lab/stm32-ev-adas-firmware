/*
 * adas.h
 *
 *  Created on: Jun 16, 2026
 *      Author: Sushant Kumar
 */

#ifndef ADAS_H
#define ADAS_H

#include <stdint.h>

typedef enum
{
    ADAS_SAFE = 0,
    ADAS_WARNING,
    ADAS_CRITICAL

} ADAS_CollisionLevel_t;

typedef struct
{
    float front_dist_cm;
    float left_dist_cm;
    float right_dist_cm;

    float speed_kmph;
    float ttc_sec;

    ADAS_CollisionLevel_t collision_level;

    uint8_t blind_left;
    uint8_t blind_right;

    uint8_t parking_assist;

    uint8_t critical_fault;

}ADAS_Data_t;

void ADAS_Init(void);

void ADAS_Update(
        float front_cm,
        float left_cm,
        float right_cm,
        float speed_kmph);

ADAS_Data_t ADAS_GetData(void);

#endif
