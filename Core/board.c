/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from the Variscite freertos-variscite SDK fork
 * (boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/board.c,
 * branch mcuxpresso_sdk_2.15.x-var02): dropped BOARD_InitDebugConsole (no
 * UART wired up in this skeleton). BOARD_InitMemory (MPU/cache regions) and
 * BOARD_RdcInit (resource domain controller: puts the M7 in its own RDC
 * domain so Linux/A53 peripheral use doesn't collide with it) are unchanged.
 */

#include "fsl_common.h"
#include "fsl_rdc.h"
#include "board.h"

/* Initialize MPU, configure memory attributes for each region. */
void BOARD_InitMemory(void)
{
    /* __CACHE_REGION_START and __CACHE_REGION_SIZE are defined in the linker file. */
    extern uint32_t __CACHE_REGION_START[];
    extern uint32_t __CACHE_REGION_SIZE[];
    uint32_t cacheStart = (uint32_t)__CACHE_REGION_START;
    uint32_t size       = (uint32_t)__CACHE_REGION_SIZE;
    uint32_t i          = 0;

    /* Disable I cache and D cache. */
    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
    {
        SCB_DisableICache();
    }
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
    {
        SCB_DisableDCache();
    }

    /* Disable MPU. */
    ARM_MPU_Disable();

    /* Region 0 [0x0000_0000 - 0x4000_0000]: Memory with Device type, not executable, not shareable, non-cacheable. */
    MPU->RBAR = ARM_MPU_RBAR(0, 0x00000000U);
    MPU->RASR = ARM_MPU_RASR(1, ARM_MPU_AP_FULL, 0, 0, 0, 1, 0, ARM_MPU_REGION_SIZE_1GB);

    /* Region 1 TCML[0x0000_0000 - 0x0001_FFFF]: Memory with Normal type, not shareable, non-cacheable. */
    MPU->RBAR = ARM_MPU_RBAR(1, 0x00000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_128KB);

    /* Region 2 QSPI[0x0800_0000 - 0x0FFF_FFFF]: Memory with Normal type, not shareable, cacheable. */
    MPU->RBAR = ARM_MPU_RBAR(2, 0x08000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_128MB);

    /* Region 3 TCMU[0x2000_0000 - 0x2002_0000]: Memory with Normal type, not shareable, non-cacheable. */
    MPU->RBAR = ARM_MPU_RBAR(3, 0x20000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_128KB);

    /* Region 4 DDR[0x4000_0000 - 0x8000_0000]: Memory with Normal type, not shareable, non-cacheable. */
    MPU->RBAR = ARM_MPU_RBAR(4, 0x40000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_1GB);

    /*
     * The DDR region [0x80000000 - 0x81000000] (16MB, see imx8mp-var-common-m7.dtsi) is
     * reserved for the CM7 core. Region 5 marks the whole 0x8000_0000-0xBFFF_FFFF window
     * non-cacheable; region 6 below re-marks the linker's text/data window cacheable,
     * matching the m_data2 window in linker/MIMX8ML8xxxxx_cm7_ram.ld.
     */
    MPU->RBAR = ARM_MPU_RBAR(5, 0x80000000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_1GB);

    while ((size >> i) > 0x1U)
    {
        i++;
    }
    if (i != 0)
    {
        assert((size & (size - 1)) == 0);
        assert(!(cacheStart % size));
        assert(size == (uint32_t)(1 << i));
        assert(i >= 5);

        /* Region 6 DDR[cacheStart]: Memory with Normal type, not shareable, cacheable. */
        MPU->RBAR = ARM_MPU_RBAR(6, cacheStart);
        MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 1, 1, 0, (i - 1));
    }

    /*
     * Enable MPU and HFNMIENA: HFNMIENA ensures the M7 core uses the MPU configuration
     * when in hard fault, NMI, and FAULTMASK handlers -- otherwise all memory regions are
     * accessed without MPU protection, risking a cacheable access to shared AIPS space.
     */
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);

    /* Enable I cache and D cache. */
    SCB_EnableICache();
    SCB_EnableDCache();
}

void BOARD_RdcInit(void)
{
    /* Move M7 core to its own RDC domain. */
    rdc_domain_assignment_t assignment = {0};
    uint8_t domainId                   = 0U;

    domainId = RDC_GetCurrentMasterDomainId(RDC);
    /* Only configure the RDC if RDC peripheral write access is allowed. */
    if ((0x1U & RDC_GetPeriphAccessPolicy(RDC, kRDC_Periph_RDC, domainId)) != 0U)
    {
        assignment.domainId = BOARD_DOMAIN_ID;
        RDC_SetMasterDomainAssignment(RDC, kRDC_Master_M7, &assignment);
    }

    /*
     * The M7 core now runs at domain 1 -- enable the clock gate for the following
     * IP/BUS/PLLs in domain 1 in the CCM, so their clocks aren't affected by the A53
     * core running at domain 0.
     */
    CLOCK_EnableClock(kCLOCK_Iomux);

    CLOCK_EnableClock(kCLOCK_Ipmux1);
    CLOCK_EnableClock(kCLOCK_Ipmux2);
    CLOCK_EnableClock(kCLOCK_Ipmux3);

    CLOCK_ControlGate(kCLOCK_SysPll1Gate, kCLOCK_ClockNeededAll);   /* Enable the CCGR gate for SysPLL1 in Domain 1 */
    CLOCK_ControlGate(kCLOCK_SysPll2Gate, kCLOCK_ClockNeededAll);   /* Enable the CCGR gate for SysPLL2 in Domain 1 */
    CLOCK_ControlGate(kCLOCK_SysPll3Gate, kCLOCK_ClockNeededAll);   /* Enable the CCGR gate for SysPLL3 in Domain 1 */
    CLOCK_ControlGate(kCLOCK_AudioPll1Gate, kCLOCK_ClockNeededAll); /* Enable the CCGR gate for AudioPLL1 in Domain 1 */
    CLOCK_ControlGate(kCLOCK_AudioPll2Gate, kCLOCK_ClockNeededAll); /* Enable the CCGR gate for AudioPLL2 in Domain 1 */
    CLOCK_ControlGate(kCLOCK_VideoPll1Gate, kCLOCK_ClockNeededAll); /* Enable the CCGR gate for VideoPLL1 in Domain 1 */
}
