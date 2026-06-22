/*
 * uart_shell.c
 *
 *  Created on: Jun 20, 2026
 *      Author: Sushant Kumar
 */


#include "uart_shell.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static UART_HandleTypeDef *shell_uart;

static uint8_t rx_byte;

#define RX_BUFFER_SIZE 128

static char rx_buffer[RX_BUFFER_SIZE];

static volatile uint16_t head = 0;
static volatile uint16_t tail = 0;

static char cmd_buffer[64];
static uint8_t cmd_index = 0;
static uint8_t cmd_ready = 0;

static void Shell_Send(const char *msg)
{
    HAL_UART_Transmit(shell_uart,
                      (uint8_t*)msg,
                      strlen(msg),
                      100);
}


uint16_t CRC16(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for(uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];

        for(uint8_t j = 0; j < 8; j++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }

    return crc;
}


void UART_Shell_Init(UART_HandleTypeDef *huart)
{
    shell_uart = huart;

    HAL_UART_Receive_IT(shell_uart, &rx_byte, 1);

    Shell_Send("\r\nUART Shell Ready\r\n");
    Shell_Send("Type: status\r\n");
}

void UART_Shell_RxCallback(void)
{
    uint16_t next_head = (head + 1) % RX_BUFFER_SIZE;

    if(next_head != tail)
    {
        rx_buffer[head] = rx_byte;
        head = next_head;
    }

    HAL_UART_Receive_IT(shell_uart, &rx_byte, 1);
}

void Process_RX_Buffer(void)
{
    while(tail != head)
    {
        char ch = rx_buffer[tail];

        tail = (tail + 1) % RX_BUFFER_SIZE;

        if(ch == '\r' || ch == '\n')
        {
            if(cmd_index > 0)
            {
                cmd_buffer[cmd_index] = '\0';
                cmd_ready = 1;
                cmd_index = 0;
            }
        }
        else
        {
            if(cmd_index < sizeof(cmd_buffer) - 1)
            {
                cmd_buffer[cmd_index++] = ch;
            }
        }
    }
}

void UART_Shell_Process(EV_Data_t *ev,
                        FaultManager_t *fault,
                        ADAS_Data_t *adas)
{
    char msg[200];

    if(!cmd_ready)
        return;

    cmd_ready = 0;

    if(strcmp(cmd_buffer, "status") == 0)
    {
        sprintf(msg,
                "\r\nSTATUS\r\nSpeed: %.1f\r\nSOC: %.1f\r\nTorque: %.1f\r\nPower: %.1f\r\nRange: %.1f\r\nADAS: %d\r\nTTC: %.2f\r\n\r\n",
                ev->speed_kmph,
                ev->soc_percent,
                ev->torque_nm,
                ev->motor_power_kw,
                ev->estimated_range_km,
                adas->collision_level,
                adas->ttc_sec);

        Shell_Send(msg);
        UART_Send_ACK();
    }
    else if(strcmp(cmd_buffer, "mode eco") == 0)
    {
        ev->mode = DRIVE_ECO;
        Shell_Send("OK: Mode ECO\r\n");
        UART_Send_ACK();
    }
    else if(strcmp(cmd_buffer, "mode normal") == 0)
    {
        ev->mode = DRIVE_NORMAL;
        Shell_Send("OK: Mode NORMAL\r\n");
        UART_Send_ACK();
    }
    else if(strcmp(cmd_buffer, "mode sport") == 0)
    {
        ev->mode = DRIVE_SPORT;
        Shell_Send("OK: Mode SPORT\r\n");
        UART_Send_ACK();
    }
    else if(strncmp(cmd_buffer, "speed ", 6) == 0)
    {
        float spd = atof(&cmd_buffer[6]);

        ev->speed_kmph = spd;

        sprintf(msg, "OK: Speed set %.1f\r\n", spd);
        Shell_Send(msg);
        UART_Send_ACK();
    }
    else if(strncmp(cmd_buffer, "soc ", 4) == 0)
    {
        float soc = atof(&cmd_buffer[4]);

        if(soc < 0)
            soc = 0;

        if(soc > 100)
            soc = 100;

        ev->soc_percent = soc;

        sprintf(msg, "OK: SOC set %.1f\r\n", soc);
        Shell_Send(msg);
        UART_Send_ACK();
    }
    else if(strcmp(cmd_buffer, "fault inject") == 0)
    {
        ev->state = VEHICLE_FAULT;

        Shell_Send("OK: Fault injected\r\n");
        UART_Send_ACK();
    }
    else if(strcmp(cmd_buffer, "fault clear") == 0)
    {
        ev->state = VEHICLE_PARKED;

        Shell_Send("OK: Fault cleared\r\n");
        UART_Send_ACK();
    }
    else if(strcmp(cmd_buffer, "reset") == 0)
    {
        EV_Init(ev);

        Shell_Send("OK: EV reset\r\n");
        UART_Send_ACK();
    }
    else
    {
        Shell_Send("ERROR: Unknown command\r\n");
    }
}

void UART_Send_EVPacket(EV_Data_t *ev)
{
    char tx[160];

    int len = sprintf(tx,
        "[01]\r\n"
        "SPD=%.1f\r\n"
        "SOC=%.1f\r\n"
        "TOR=%.1f\r\n"
        "PWR=%.1f\r\n"
        "RNG=%.1f\r\n",
        ev->speed_kmph,
        ev->soc_percent,
        ev->torque_nm,
        ev->motor_power_kw,
        ev->estimated_range_km
    );

    uint16_t crc = CRC16((uint8_t*)tx, len);

    sprintf(tx + len, "\r\nCRC=%04X\r\n", crc);

    Shell_Send(tx);
}


void UART_Send_ADASPacket(ADAS_Data_t *adas)
{
    char tx[160];

    int len = sprintf(tx,
        "[02]\r\n"
        "F=%.2f\r\n"
        "L=%.2f\r\n"
        "R=%.2f\r\n"
        "COL=%d\r\n"
        "BL=%d\r\n"
        "BR=%d\r\n"
        "TTC=%.1f\r\n",
        adas->front_dist_cm,
        adas->left_dist_cm,
        adas->right_dist_cm,
        adas->collision_level,
        adas->blind_left,
        adas->blind_right,
        adas->ttc_sec
    );

    uint16_t crc = CRC16((uint8_t*)tx, len);

    sprintf(tx + len, "\r\nCRC=%04X\r\n", crc);

    Shell_Send(tx);
}


void UART_Send_StatePacket(VehicleState_t state)
{
    char tx[64];
    const char *name;

    switch(state)
    {
        case VEHICLE_PARKED:  name = "PARKED";  break;
        case VEHICLE_READY:   name = "READY";   break;
        case VEHICLE_DRIVING: name = "DRIVING"; break;
        case VEHICLE_REGEN:   name = "REGEN";   break;
        case VEHICLE_FAULT:   name = "FAULT";   break;
        default:              name = "UNKNOWN"; break;
    }

    sprintf(tx,
            "\r\n[03]\r\n"
            "STATE=%s\r\n",
            name);

    Shell_Send(tx);
}

void UART_Send_ACK(void)
{
    Shell_Send("\r\n[04]\r\nACK\r\n");
}
