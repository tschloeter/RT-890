/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "app/uart.h"
#include "bsp/gpio.h"
#include "driver/pins.h"
#include "driver/serial-flash.h"
#include "driver/uart.h"
#include "radio/hardware.h"
#include "radio/settings.h"

static uint8_t volatile region = 0;
static bool volatile bFlashing = false;
uint16_t volatile UART_Timer = 0;
bool volatile UART_IsRunning = false;

static uint8_t checksum(uint8_t const *data, uint8_t size)
{
	uint8_t sum = 0;

	while (size--) {
		sum += *data++;
	}

	return sum;
}

static uint8_t execute_flash_command(uint8_t cmd, uint16_t block, uint8_t *data)
{
	static uint16_t page_erased = 0xFFFFu;
	static uint32_t blocks_written = 0;
	uint16_t page = 0;
	
	if (cmd == 0x52) {
		SFLASH_Read(data, block * SFLASH_BLOCK_SIZE, SFLASH_BLOCK_SIZE);
		return RESPONSE_OK;
	}
	
	TMR1->ctrl1_bit.tmren = FALSE;
	bFlashing = true;

	// Flash region write commands, parameters are block number (128 bytes per block)
	uint16_t page_offset = block / (SFLASH_PAGE_SIZE / SFLASH_BLOCK_SIZE);
	block %= (SFLASH_PAGE_SIZE / SFLASH_BLOCK_SIZE);
	uint16_t pages_in_region = 0;
	switch (cmd) {
	case 0x40: page = 0x000u + page_offset; region = 0; pages_in_region = 0x02D0u; break;
	case 0x41: page = 0x2D0u + page_offset; region = 0; pages_in_region = 0x0028u; break;
	case 0x42: page = 0x2F8u + page_offset; region = 0; pages_in_region = 0x0022u; break;
	case 0x43: page = 0x31Au + page_offset; region = 0; pages_in_region = 0x0002u; break;
	case 0x47: page = 0x3B5u + page_offset; region = 0; pages_in_region = 0x000Au; break;
	case 0x48: page = 0x3BFu + page_offset; region = 1; pages_in_region = 0x0001u; break;
	case 0x49: page = 0x3C1u + page_offset; region = 2; pages_in_region = 0x000Au; break;
	case 0x4B: page = 0x3D8u + page_offset; region = 0; pages_in_region = 0x000Au; break;
	case 0x4C: page = 0x31Cu + page_offset; region = 0; pages_in_region = 0x0099u; break;
	case 0x57: page = 0x000u + page_offset; region = 0; pages_in_region = 0x0400u; break;
	default:
		return RESPONSE_NOK;
	}

	if (page_offset > pages_in_region - 1) {
		return RESPONSE_NOK;
	}
	
	if (page >= SFLASH_SIZE / SFLASH_PAGE_SIZE) {
		return RESPONSE_NOK;
	}

	if (page != page_erased) {
		SFLASH_Erase(page);
		page_erased = page;
		blocks_written = 0;
	}
	else if (blocks_written & (1 << block)) {
		return RESPONSE_NOK;
	}
	SFLASH_Write(data, (page * SFLASH_PAGE_SIZE) + (block * SFLASH_BLOCK_SIZE), SFLASH_BLOCK_SIZE);
	blocks_written |= (1 << block);
	return RESPONSE_OK;
}

void HandlerUSART1(void)
{
	static uint8_t Buffer[256] = { 0 };
	static uint8_t BufferLength = 0;

	if (USART1->ctrl1_bit.rdbfien && (USART1->sts & USART_RDBF_FLAG)) {
		uint8_t cmd;

		gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);

		Buffer[BufferLength++] = USART1->dt;
		BufferLength %= 256;
		UART_IsRunning = true;
		UART_Timer = 1000;
		
		// Command started, check if it's valid
		cmd = Buffer[0];
		if (1 == BufferLength) {
			switch (cmd) {
				case 0x35: // Backup/Restore
				case 0x32: // NOP
				case 0x40:
				case 0x41:
				case 0x42:
				case 0x43:
				case 0x47:
				case 0x48:
				case 0x49:
				case 0x4B:
				case 0x4C:
				case 0x52: // Read Flash
				case 0x57: // Write Flash
					break;

				default:
					UART_SendByte(RESPONSE_NOK);
					UART_IsRunning = false;
					UART_Timer = 0;
					BufferLength = 0;
			}	
		} else { // cmd accumulating in buffer, check if we have a complete command
		
			switch(cmd) {
				case 0x32:
					if (5 == BufferLength) {
						if (checksum(Buffer, BufferLength - 1) != Buffer[BufferLength - 1]) {
							UART_SendByte(RESPONSE_NOK);
						} else {
							if (Buffer[3] == 0x16 || Buffer[3] == 0x10) {
								UART_SendByte(RESPONSE_OK);
							}
						}
						UART_IsRunning = false;
						UART_Timer = 0;
						BufferLength = 0;
					}
					break;

				case 0x35:
					if (5 == BufferLength) {
						if (checksum(Buffer, BufferLength - 1) != Buffer[BufferLength - 1]) {
							UART_SendByte(RESPONSE_NOK);
						} else {
							switch (Buffer[3]) {
							case 0x16:
								UART_SendByte(RESPONSE_OK);
								break;

							case 0xEE:
								if (bFlashing) {
									if (region == 1) {
										SETTINGS_BackupCalibration();
										UART_SendByte(RESPONSE_OK);
									} else if (region == 2) {
										SETTINGS_BackupSettings();
										UART_SendByte(RESPONSE_OK);
									}
									else {
										UART_SendByte(RESPONSE_NOK);
									}
									HARDWARE_Reboot();
									for(;;);
								}
								break;

							default:
								UART_SendByte(RESPONSE_NOK);
							}
						}
						UART_IsRunning = false;
						UART_Timer = 0;
						BufferLength = 0;
					}
					break;

				case 0x40:
				case 0x41:
				case 0x42:
				case 0x43:
				case 0x47:
				case 0x48:
				case 0x49:
				case 0x4B:
				case 0x4C:
				case 0x57:
					if (132 == BufferLength) {
						if (checksum(Buffer, BufferLength - 1) != Buffer[BufferLength - 1]) {
							UART_SendByte(RESPONSE_NOK);
						} else {
							UART_SendByte(execute_flash_command(cmd, (((uint16_t) Buffer[1] << 8) | ((uint16_t) Buffer[2])), Buffer + 3));
						}
						BufferLength = 0;
					}
					break;

				case 0x52:
					if (4 == BufferLength) {
						if (checksum(Buffer, BufferLength - 1) != Buffer[BufferLength - 1]) {
							UART_SendByte(RESPONSE_NOK);
						} else {
							if (RESPONSE_OK == execute_flash_command(cmd, (((uint16_t) Buffer[1] << 8) | ((uint16_t) Buffer[2])), Buffer + 3)) {
								Buffer[131] = checksum(Buffer, 131);
								UART_Send(Buffer, 132);
							} else {
								UART_SendByte(RESPONSE_NOK);
							}
						}
						UART_IsRunning = false;
						UART_Timer = 0;
						BufferLength = 0;
					}
					break;

				default:
					UART_SendByte(RESPONSE_NOK);
					UART_IsRunning = false;
					UART_Timer = 0;
					BufferLength = 0;
			}				
		}
	}

	if (bFlashing) {
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
	}
	else if (UART_IsRunning) {
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	}
	else {
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
	}
}

