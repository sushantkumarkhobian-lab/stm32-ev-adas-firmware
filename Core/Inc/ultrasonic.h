/*
 * ultrasinic.h
 *
 *  Created on: Jun 16, 2026
 *      Author: Sushant Kumar
 */

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "main.h"

void HCSR04_Init(void);
float HCSR04_Read(uint8_t sensor);
void HCSR04_ReadAll(float distance[3]);

#endif
