/*
 * Copyright (C) ST-Ericsson SA 2010
 *
 * Author: Rabin Vincent <rabin.vincent@stericsson.com>
 * License terms: GNU General Public License (GPL) version 2
 */

#ifndef __MACH_IRQS_DB8500_H
#define __MACH_IRQS_DB8500_H

#define IRQ_DB8500_MTU0			(IRQ_SHPI_START + 4)
#define IRQ_DB8500_SPI2			(IRQ_SHPI_START + 6)
#define IRQ_DB8500_PMU			(IRQ_SHPI_START + 7)
#define IRQ_DB8500_SPI0			(IRQ_SHPI_START + 8)
#define IRQ_DB8500_RTT			(IRQ_SHPI_START + 9)
#define IRQ_DB8500_PKA			(IRQ_SHPI_START + 10)
#define IRQ_DB8500_UART0		(IRQ_SHPI_START + 11)
#define IRQ_DB8500_I2C3			(IRQ_SHPI_START + 12)
#define IRQ_DB8500_L2CC			(IRQ_SHPI_START + 13)
#define IRQ_DB8500_SSP0			(IRQ_SHPI_START + 14)
#define IRQ_DB8500_CRYP1		(IRQ_SHPI_START + 15)
#define IRQ_DB8500_MSP1_RX		(IRQ_SHPI_START + 16)
#define IRQ_DB8500_MTU1			(IRQ_SHPI_START + 17)
#define IRQ_DB8500_RTC			(IRQ_SHPI_START + 18)
#define IRQ_DB8500_UART1		(IRQ_SHPI_START + 19)
#define IRQ_DB8500_USB_WAKEUP		(IRQ_SHPI_START + 20)
#define IRQ_DB8500_I2C0			(IRQ_SHPI_START + 21)
#define IRQ_DB8500_I2C1			(IRQ_SHPI_START + 22)
#define IRQ_DB8500_USBOTG		(IRQ_SHPI_START + 23)
#define IRQ_DB8500_DMA_SECURE		(IRQ_SHPI_START + 24)
#define IRQ_DB8500_DMA			(IRQ_SHPI_START + 25)
#define IRQ_DB8500_UART2		(IRQ_SHPI_START + 26)
#define IRQ_DB8500_ICN_PMU1		(IRQ_SHPI_START + 27)
#define IRQ_DB8500_ICN_PMU2		(IRQ_SHPI_START + 28)
#define IRQ_DB8500_HSIR_EXCEP		(IRQ_SHPI_START + 29)
#define IRQ_DB8500_MSP0			(IRQ_SHPI_START + 31)
#define IRQ_DB8500_HSIR_CH0_OVRRUN	(IRQ_SHPI_START + 32)
#define IRQ_DB8500_HSIR_CH1_OVRRUN	(IRQ_SHPI_START + 33)
#define IRQ_DB8500_HSIR_CH2_OVRRUN	(IRQ_SHPI_START + 34)
#define IRQ_DB8500_HSIR_CH3_OVRRUN	(IRQ_SHPI_START + 35)
#define IRQ_DB8500_HSIR_CH4_OVRRUN	(IRQ_SHPI_START + 36)
#define IRQ_DB8500_HSIR_CH5_OVRRUN	(IRQ_SHPI_START + 37)
#define IRQ_DB8500_HSIR_CH6_OVRRUN	(IRQ_SHPI_START + 38)
#define IRQ_DB8500_HSIR_CH7_OVRRUN	(IRQ_SHPI_START + 39)
#define IRQ_DB8500_AB8500		(IRQ_SHPI_START + 40)
#define IRQ_DB8500_SDMMC2		(IRQ_SHPI_START + 41)
#define IRQ_DB8500_SIA			(IRQ_SHPI_START + 42)
#define IRQ_DB8500_SIA2			(IRQ_SHPI_START + 43)
#define IRQ_DB8500_SVA			(IRQ_SHPI_START + 44)
#define IRQ_DB8500_SVA2			(IRQ_SHPI_START + 45)
#define IRQ_DB8500_PRCMU0		(IRQ_SHPI_START + 46)
#define IRQ_DB8500_PRCMU1		(IRQ_SHPI_START + 47)
#define IRQ_DB8500_DISP			(IRQ_SHPI_START + 48)
#define IRQ_DB8500_SPI3			(IRQ_SHPI_START + 49)
#define IRQ_DB8500_SDMMC1		(IRQ_SHPI_START + 50)
#define IRQ_DB8500_I2C4			(IRQ_SHPI_START + 51)
#define IRQ_DB8500_SSP1			(IRQ_SHPI_START + 52)
#define IRQ_DB8500_SKE			(IRQ_SHPI_START + 53)
#define IRQ_DB8500_KB			(IRQ_SHPI_START + 54)
#define IRQ_DB8500_I2C2			(IRQ_SHPI_START + 55)
#define IRQ_DB8500_B2R2			(IRQ_SHPI_START + 56)
#define IRQ_DB8500_CRYP0		(IRQ_SHPI_START + 57)
#define IRQ_DB8500_SDMMC3		(IRQ_SHPI_START + 59)
#define IRQ_DB8500_SDMMC0		(IRQ_SHPI_START + 60)
#define IRQ_DB8500_HSEM			(IRQ_SHPI_START + 61)
#define IRQ_DB8500_MSP1			(IRQ_SHPI_START + 62)
#define IRQ_DB8500_SBAG			(IRQ_SHPI_START + 63)
#define IRQ_DB8500_SPI1			(IRQ_SHPI_START + 96)
#define IRQ_DB8500_SRPTIMER		(IRQ_SHPI_START + 97)
#define IRQ_DB8500_MSP2			(IRQ_SHPI_START + 98)
#define IRQ_DB8500_SDMMC4		(IRQ_SHPI_START + 99)
#define IRQ_DB8500_SDMMC5		(IRQ_SHPI_START + 100)
#define IRQ_DB8500_HSIRD0		(IRQ_SHPI_START + 104)
#define IRQ_DB8500_HSIRD1		(IRQ_SHPI_START + 105)
#define IRQ_DB8500_HSITD0		(IRQ_SHPI_START + 106)
#define IRQ_DB8500_HSITD1		(IRQ_SHPI_START + 107)
#define IRQ_DB8500_CTI0			(IRQ_SHPI_START + 108)
#define IRQ_DB8500_CTI1			(IRQ_SHPI_START + 109)
#define IRQ_DB8500_ICN_ERR		(IRQ_SHPI_START + 110)
#define IRQ_DB8500_MALI_PPMMU		(IRQ_SHPI_START + 112)
#define IRQ_DB8500_MALI_PP		(IRQ_SHPI_START + 113)
#define IRQ_DB8500_MALI_GPMMU		(IRQ_SHPI_START + 114)
#define IRQ_DB8500_MALI_GP		(IRQ_SHPI_START + 115)
#define IRQ_DB8500_MALI			(IRQ_SHPI_START + 116)
#define IRQ_DB8500_PRCMU_SEM		(IRQ_SHPI_START + 118)
#define IRQ_DB8500_GPIO0		(IRQ_SHPI_START + 119)
#define IRQ_DB8500_GPIO1		(IRQ_SHPI_START + 120)
#define IRQ_DB8500_GPIO2		(IRQ_SHPI_START + 121)
#define IRQ_DB8500_GPIO3		(IRQ_SHPI_START + 122)
#define IRQ_DB8500_GPIO4		(IRQ_SHPI_START + 123)
#define IRQ_DB8500_GPIO5		(IRQ_SHPI_START + 124)
#define IRQ_DB8500_GPIO6		(IRQ_SHPI_START + 125)
#define IRQ_DB8500_GPIO7		(IRQ_SHPI_START + 126)
#define IRQ_DB8500_GPIO8		(IRQ_SHPI_START + 127)

#endif
