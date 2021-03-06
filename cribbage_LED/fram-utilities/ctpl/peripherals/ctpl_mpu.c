/* --COPYRIGHT--,FRAM-Utilities
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * This source code is part of FRAM Utilities for MSP430 FRAM Microcontrollers.
 * Visit http://www.ti.com/tool/msp-fram-utilities for software information and
 * download.
 * --/COPYRIGHT--*/
#include <msp430.h>

#if defined(__MSP430FR2XX_4XX_FAMILY__) && !defined(__AUTOGENERATED__)
#include <msp430fr2xx_4xxgeneric.h>
#elif defined(__MSP430FR57XX_FAMILY__) && !defined(__AUTOGENERATED__)
#include <msp430fr57xxgeneric.h>
#elif defined(__MSP430FR5XX_6XX_FAMILY__) && !defined(__AUTOGENERATED__)
#include <msp430fr5xx_6xxgeneric.h>
#endif

#ifdef __MSP430_HAS_MPU__

#include <stdint.h>

#include "ctpl_mpu.h"
#include "ctpl_hwreg.h"
#include "../ctpl_low_level.h"

void ctpl_MPU_save(uint16_t baseAddress, uint16_t *storage, uint16_t mode)
{
    /* Save register context to non-volatile storage. */
#if defined(__MSP430FR57XX_FAMILY__)    
    storage[2] = HWREG16(baseAddress + OFS_MPUCTL0);
    storage[1] = HWREG16(baseAddress + OFS_MPUSAM);
    storage[0] = HWREG16(baseAddress + OFS_MPUSEG);
#elif defined(__MSP430FR5XX_6XX_FAMILY__)
    storage[6] = HWREG16(baseAddress + OFS_MPUCTL0);
    storage[5] = HWREG16(baseAddress + OFS_MPUIPC0);
    storage[4] = HWREG16(baseAddress + OFS_MPUIPSEGB1);
    storage[3] = HWREG16(baseAddress + OFS_MPUIPSEGB2);
    storage[2] = HWREG16(baseAddress + OFS_MPUSAM);
    storage[1] = HWREG16(baseAddress + OFS_MPUSEGB1);
    storage[0] = HWREG16(baseAddress + OFS_MPUSEGB2);
#else
    #error Unsupported device family for MPU.
#endif

    return;
}

void ctpl_MPU_restore(uint16_t baseAddress, uint16_t *storage, uint16_t mode)
{
    /* Restore register context from non-volatile storage. */
#if defined(__MSP430FR57XX_FAMILY__)
    HWREG8(baseAddress + OFS_MPUCTL0_H) = MPUPW_H;
    HWREG16(baseAddress + OFS_MPUSEG) = storage[0];
    HWREG16(baseAddress + OFS_MPUSAM) = storage[1];
    HWREG8(baseAddress + OFS_MPUCTL0_L) = storage[2] & 0x00ff;
    HWREG8(baseAddress + OFS_MPUCTL0_H) = 0;
#elif defined(__MSP430FR5XX_6XX_FAMILY__)
    HWREG8(baseAddress + OFS_MPUCTL0_H) = MPUPW_H;
    HWREG16(baseAddress + OFS_MPUSEGB2) = storage[0];
    HWREG16(baseAddress + OFS_MPUSEGB1) = storage[1];
    HWREG16(baseAddress + OFS_MPUSAM) = storage[2];
    HWREG16(baseAddress + OFS_MPUIPSEGB2) = storage[3];
    HWREG16(baseAddress + OFS_MPUIPSEGB1) = storage[4];
    HWREG16(baseAddress + OFS_MPUIPC0) = storage[5];
    HWREG8(baseAddress + OFS_MPUCTL0_L) = storage[6] & 0x00ff;
    HWREG8(baseAddress + OFS_MPUCTL0_H) = 0;
#else
    #error Unsupported device family for MPU.
#endif

    return;
}

#endif //__MSP430_HAS_MPU__
