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

#include "app/radio.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "driver/serial-flash.h"
#include "helper/dtmf.h"
#include "misc.h"
#include "radio/hardware.h"
#include "radio/settings.h"
#include "task/keyaction.h"
#include "task/scanner.h"
#include "ui/gfx.h"
#include "ui/helper.h"

Calibration_t gCalibration;
char gDeviceName[16];
gSettings_t gSettings;
char WelcomeString[32];
uint32_t gFrequencyStep = 25;
gExtendedSettings_t gExtendedSettings;

static void RestoreCalibration(void)
{
	SFLASH_Read(gFlashBuffer, BASE_ADDR_CALIBRATION_BACKUP, 0x1000);
	SFLASH_Update(gFlashBuffer, BASE_ADDR_CALIBRATION_ACTIVE, 0x1000);
}

void SETTINGS_BackupCalibration(void)
{
	SFLASH_Read(gFlashBuffer, BASE_ADDR_CALIBRATION_ACTIVE, 0x1000);
	SFLASH_Update(gFlashBuffer, BASE_ADDR_CALIBRATION_BACKUP, 0x1000);
}

void SETTINGS_LoadCalibration(void)
{
	if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
		if (KEY_GetButton() == KEY_EXIT) {
			gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
			gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
			RestoreCalibration();
			while (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
			}
			gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
			gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
			HARDWARE_Reboot();
		}
	}

	SFLASH_Read(&gCalibration, BASE_ADDR_CALIBRATION_ACTIVE, 0x20);
	if (gCalibration._0x00 != 0x9A) {
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
		while (1) {
		}
	}

	if (gCalibration._0x01 != 1) {
		if (gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT) || KEY_GetButton() != KEY_7) {
			return;
		}
	}

	gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
	gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
	gpio_bits_set(GPIOA, BOARD_GPIOA_LCD_RESX);

	while (1) {
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(0, 0, 0)); //Black
		DELAY_WaitMS(1000);
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(31, 63, 31)); //White
		DELAY_WaitMS(1000);
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(31, 0, 0)); //Red
		DELAY_WaitMS(1000);
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(0, 32, 0)); //Green
		DELAY_WaitMS(1000);
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(0, 0, 31)); //Blue
		DELAY_WaitMS(1000);
	}
}

void SETTINGS_LoadSettings(void)
{
	SFLASH_Read(WelcomeString, BASE_ADDR_WELCOME_STRING, sizeof(WelcomeString));
	SFLASH_Read(gDeviceName, BASE_ADDR_DEVICE_NAME, sizeof(gDeviceName));
	SFLASH_Read(&gSettings, BASE_ADDR_SETTINGS, sizeof(gSettings));
	SFLASH_Read(&gDTMF_Settings, BASE_ADDR_DTMF_SETTINGS, sizeof(gDTMF_Settings));
	SFLASH_Read(&gDTMF_Contacts, BASE_ADDR_DTMF_CONTACTS, sizeof(gDTMF_Contacts));
	SFLASH_Read(&gDTMF_Kill, BASE_ADDR_DTMF_KILL, sizeof(gDTMF_Kill));
	SFLASH_Read(&gDTMF_Stun, BASE_ADDR_DTMF_STUN, sizeof(gDTMF_Stun));
	SFLASH_Read(&gDTMF_Wake, BASE_ADDR_DTMF_WAKE, sizeof(gDTMF_Wake));
	// Extended Settings bits are all 1 at first read as the flash is full of 0xFF
	SFLASH_Read(&gExtendedSettings, BASE_ADDR_EXTENDED_SETTINGS, sizeof(gExtendedSettings));

	if (gExtendedSettings.KeyShortcut[0] == 0xFF) {
		SetDefaultKeyShortcuts(false); //
	}

	if (gExtendedSettings.SqGlitchBase == 0xFF) {
		gExtendedSettings.SqRSSIBase = 0x5E;
		gExtendedSettings.SqNoiseBase = 0x44;
		gExtendedSettings.SqGlitchBase = 0x11;
	}
	if (gExtendedSettings.ScanDelay == 63) {
		gExtendedSettings.ScanDelay = 35;
	}

	gFrequencyStep = FREQUENCY_GetStep(gSettings.FrequencyStep);

	UI_SetColors(gExtendedSettings.DarkMode);

	if (gExtendedSettings.MicGainLevel > 31) {
		gExtendedSettings.MicGainLevel = 19;
	}
	BK4819_SetMicSensitivityTuning();

	gSettings.bEnableDisplay = 1;
	if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
		if (KEY_GetButton() == KEY_HASH) {
			gSettings.bFLock ^= 1;
			SETTINGS_SaveGlobals();
			do {
				while (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
				}
			} while (KEY_GetButton() == KEY_HASH);
			KEY_Side2Counter = 0;
			KEY_KeyCounter = 0;
		}
	}

#ifdef DISALLOW_TRANSMIT
	gSettings.RepeaterMode = 0;
	gSettings.Vox = 0;
#endif
}

void SETTINGS_SaveGlobals(void)
{
	SFLASH_Update(&gSettings, BASE_ADDR_SETTINGS, sizeof(gSettings));
	SFLASH_Update(&gExtendedSettings, BASE_ADDR_EXTENDED_SETTINGS, sizeof(gExtendedSettings));
}

void SETTINGS_SaveState(void)
{
	gScannerMode = false;
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	SCANNER_Countdown = 0;
	if (gSettings.WorkMode) {
		SETTINGS_SaveGlobals();
	} else {
		CHANNELS_SaveChannel(gSettings.CurrentVfo ? 1000 : 999, &gVfoState[gSettings.CurrentVfo]);
	}
	UI_DrawScan();
}

void SETTINGS_SaveDTMF(void)
{
	SFLASH_Update(&gDTMF_Settings, BASE_ADDR_DTMF_SETTINGS, sizeof(gDTMF_Settings));
}

void SETTINGS_FactoryReset(void)
{
	uint8_t Lock;
	uint8_t page;

	Lock = gSettings.bFLock;
	for (page = 0; page < 10; page++) {
		SFLASH_Read(gFlashBuffer, BASE_ADDR_SETTINGS_FACTORY + (page * SFLASH_PAGE_SIZE), SFLASH_PAGE_SIZE);
		SFLASH_Update(gFlashBuffer, BASE_ADDR_SETTINGS_ACTIVE + (page * SFLASH_PAGE_SIZE), SFLASH_PAGE_SIZE);
	}
	SFLASH_Read(&gSettings, BASE_ADDR_SETTINGS, sizeof(gSettings));
	gSettings.bFLock = Lock;
	SETTINGS_SaveGlobals();
}

void SETTINGS_SaveDeviceName(void)
{
	SFLASH_Update(gDeviceName, BASE_ADDR_DEVICE_NAME, sizeof(gDeviceName));
}

void SETTINGS_BackupSettings(void)
{
	uint8_t page;
	for (page = 0; page < 10; page++) {
		SFLASH_Read(gFlashBuffer, BASE_ADDR_SETTINGS_ACTIVE + (page * SFLASH_PAGE_SIZE), SFLASH_PAGE_SIZE);
		SFLASH_Update(gFlashBuffer, BASE_ADDR_SETTINGS_FACTORY + (page * SFLASH_PAGE_SIZE), SFLASH_PAGE_SIZE);
	}
}

