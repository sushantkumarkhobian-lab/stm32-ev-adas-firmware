/*
 * uart_shell.h
 *
 *  Created on: Jun 20, 2026
 *      Author: Sushant Kumar
 */

#ifndef UART_SHELL_H
#define UART_SHELL_H

#include "main.h"
#include "ev_control.h"
#include "fault.h"
#include "adas.h"


void UART_Send_EVPacket(EV_Data_t *ev);

void UART_Send_ADASPacket(ADAS_Data_t *adas);

void UART_Send_StatePacket(VehicleState_t state);

void UART_Send_ACK(void);

void UART_Shell_Init(UART_HandleTypeDef *huart);

void Process_RX_Buffer(void);

void UART_Shell_Process(EV_Data_t *ev,
                        FaultManager_t *fault,
                        ADAS_Data_t *adas);

void UART_Shell_RxCallback(void);

#endif
