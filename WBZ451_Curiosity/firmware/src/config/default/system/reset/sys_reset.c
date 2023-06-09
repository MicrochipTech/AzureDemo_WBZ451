/*******************************************************************************
  Reset System Service Source File

  Company:
    Microchip Technology Inc.

  File Name:
    sys_reset.c

  Summary:
    Reset System Service source file.

  Description:
    This source file contains the function implementations of the APIs
    supported by the module.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/******************************************************************************
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
//DOM-IGNORE-END

#include "device.h"
#include "system/reset/sys_reset.h"

void __attribute__((noreturn)) SYS_RESET_SoftwareReset(void) {
    CFG_REGS->CFG_SYSKEY = 0x00000000;
    CFG_REGS->CFG_SYSKEY = 0xAA996655U;
    CFG_REGS->CFG_SYSKEY = 0x556699AA;
    RCON_REGS->RCON_RSWRSTSET = RCON_RSWRST_SWRST_Msk;

    /* Read RSWRST register to trigger reset */
    RCON_REGS->RCON_RSWRST;

    /* Prevent any unwanted code execution until reset occurs */
    while (1) {
    }
}

/*******************************************************************************
 End of File
 */
