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

#ifndef DRIVER_SERIAL_FLASH_H
#define DRIVER_SERIAL_FLASH_H

#include <stdint.h>

#define SFLASH_PAGE_SIZE                    (0x1000)
#define BASE_ADDR_AUDIO_MENU				(0x000000)
#define BASE_ADDR_AUDIO_DIGITS_SHORT        (0x118000)
#define BASE_ADDR_AUDIO_DIGITS_LONG         (0x188000)
#define BASE_ADDR_RESERVED1                 (0x1AC000)
#define BASE_ADDR_UNKNOWN                   (0x2D0000)
#define BASE_ADDR_RESERVED2                 (0x2F5000)
#define BASE_ADDR_FONT_EXTENDED             (0x2F8000)
#define BASE_ADDR_RESERVED3                 (0x319000)
#define BASE_ADDR_FONT_STANDARD             (0x31A000)
#define BASE_ADDR_RESERVED4                 (0x3AA000)
#define BASE_ADDR_IMAGE_LOGO				(0x3B5000)
#define BASE_ADDR_CALIBRATION_ACTIVE	    (0x3BF000)
#define BASE_ADDR_FREQUENCY_BAND_INFO		(0x3BF020)
#define BASE_ADDR_RESERVED5                 (0x3BFD00)
#define BASE_ADDR_CALIBRATION_BACKUP	    (0x3C0000)
#define BASE_ADDR_RESERVED6                 (0x3C0D00)
#define BASE_ADDR_SETTINGS_ACTIVE           BASE_ADDR_WELCOME_STRING
#define BASE_ADDR_WELCOME_STRING			(0x3C1000)
#define BASE_ADDR_DEVICE_NAME				(0x3C1020)
#define BASE_ADDR_SETTINGS	                (0x3C1030)
#define BASE_ADDR_CHANNELS					(0x3C2000)
#define BASE_ADDR_DTMF_SETTINGS	            (0x3C9D20)
#define BASE_ADDR_DTMF_CONTACTS	            (0x3C9D30)
#define BASE_ADDR_DTMF_KILL		            (0x3C9E30)
#define BASE_ADDR_DTMF_STUN		            (0x3C9E40)
#define BASE_ADDR_DTMF_WAKE		            (0x3C9E50)
#define BASE_ADDR_SETTINGS_FACTORY	        (0x3CB000)
#define BASE_ADDR_EXTENDED_SETTINGS	        (0x3D5000)

void SFLASH_Init(void);
void SFLASH_Read(void *pBuffer, uint32_t Address, uint16_t Size);
void SFLASH_Erase(uint32_t Page);
void SFLASH_Write(const void *pBuffer, uint32_t Address, uint16_t Size);
void SFLASH_Update(const void *pBuffer, uint32_t Address, uint16_t Size);

#endif

