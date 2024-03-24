/**
 * @file fc4xxx_driver_scg.c
 * @author Flagchip
 * @brief FC4xxx SCG driver type definition and API
 * @version 0.1.0B
 * @date 2023-09-07
 *
 * @copyright Copyright (c) 2020-2023 Flagchip Semiconductors Co., Ltd.
 *
 */

/********************************************************************************
*   Revision History:
*
*   Version     Date          Initials       CR#          Descriptions
*   ---------   ----------    ------------   ----------   ---------------
*   0.1.0       2023-08-21    Flagchip031    N/A          FC4150F1M release version 0.1.0
*   0.1.0B      2023-09-07    Flagchip032    N/A          FC4150F1MB release version 0.1.0B
********************************************************************************/
#include "fc4xxx_driver_scg.h"


/* ################################################################################## */
/* ####################################### Macro #################################### */
#define SIRC_CLOCK       12000000U
#define SIRC32K_CLOCK    32000U
#define FIRC_CLOCK       96000000U
#define FOSC_STABILIZATION_TIMEOUT 320500U
#define FIRC_STABILIZATION_TIMEOUT 20U
#define SIRC_STABILIZATION_TIMEOUT 100U
#define SOSC_STABILIZATION_TIMEOUT 320500U
#define PLL0_STABILIZATION_TIMEOUT 320500U
#define SCG_CLKSRC_STABILIZATION_TIMEOUT 1000U
#define CLOCK_OFF_STABILIZATION_TIMEOUT 1000U
#define CLOCK_DIV_STABILIZATION_TIMEOUT 1000U
#define PLL0_CLK_MAX 200000000U
#define PLL0_CLK_MIN 75000000U
#define SYS_CORE_CLK_MAX 150000000U
#define SYS_BUS_CLK_MAX 75000000U
#define SYS_SLOW_CLK_MAX 37500000U
#define ASYNC_CLOCKDIVL_DEFAULT  SCG_ASYNCCLOCKDIV_BY8

/* ################################################################################## */
/* ################################ Local Variables ################################# */
static SCG_ClockSequenceType s_tClockSequenceInfo =
{
    .eRunClock = SCG_RUNCLOCK_NONE,
};

static SCG_CLockError_CallBackType s_SircClkErrNotify;
static SCG_CLockError_CallBackType s_SoscClkErrNotify;
static SCG_CLockError_CallBackType s_FoscClkErrNotify;
static SCG_CLockError_CallBackType s_Pll0ClkErrNotify;
static SCG_CLockError_CallBackType s_FircClkErrNotify;


/* ################################################################################## */
/* ########################### Local Prototype Functions ############################ */


/* ################################################################################## */
/* ######################### Global prototype Functions  ############################ */

/* ################################################################################## */
/* ################################ Local Functions  ################################ */

static void SCG_SetFircClockStatus(void)
{
    bool bStatus;
    uint32_t u32DivRegVal = 0U;
    /* check if FIRC is valid, then set s_tScgClockInfo[SCG_FIRC_CLK] to valid value */
    bStatus = SCG_HWA_GetFircValid();
    if (bStatus == true)
    {
        s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].eClkStatus = SCG_CLOCK_VALID;
        s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq = FIRC_CLOCK;
        u32DivRegVal = SCG_HWA_GetFircDiv();
        s_tClockSequenceInfo.tClockInfo[SCG_FIRCDIVH_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVH_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_FIRCDIVH_CLK].u32Freq = SCG_CALCULATE_DIVH_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq, u32DivRegVal);

        s_tClockSequenceInfo.tClockInfo[SCG_FIRCDIVM_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVM_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_FIRCDIVM_CLK].u32Freq = SCG_CALCULATE_DIVM_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq, u32DivRegVal);
    }
    else
    {
        s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_FIRCDIVH_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_FIRCDIVH_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_FIRCDIVM_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_FIRCDIVM_CLK].u32Freq = 0U;
    }
}

static void SCG_SetSircClockStatus(void)
{
    bool bStatus;
    uint32_t u32DivRegVal = 0U;

    /* check if SIRC is valid, then set s_tScgClockInfo[SCG_SIRC_CLK] to valid value */
    bStatus = SCG_HWA_GetSircValid();
    if (bStatus == true)
    {
        s_tClockSequenceInfo.tClockInfo[SCG_SIRC_CLK].eClkStatus = SCG_CLOCK_VALID;
        s_tClockSequenceInfo.tClockInfo[SCG_SIRC_CLK].u32Freq = SIRC_CLOCK;
        u32DivRegVal = SCG_HWA_GetSircDiv();
        s_tClockSequenceInfo.tClockInfo[SCG_SIRCDIVH_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVH_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_SIRCDIVH_CLK].u32Freq = SCG_CALCULATE_DIVH_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_SIRC_CLK].u32Freq, u32DivRegVal);

        s_tClockSequenceInfo.tClockInfo[SCG_SIRCDIVM_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVM_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_SIRCDIVM_CLK].u32Freq = SCG_CALCULATE_DIVM_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_SIRC_CLK].u32Freq, u32DivRegVal);
    }
    else
    {
        s_tClockSequenceInfo.tClockInfo[SCG_SIRC_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_SIRC_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_SIRCDIVH_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_SIRCDIVH_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_SIRCDIVM_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_SIRCDIVM_CLK].u32Freq = 0U;
    }
}

static void SCG_SetSirc32kClockStatus(void)
{
    bool bStatus;

    /* check if SIRC32K is valid, then set s_tScgClockInfo[SCG_SIRC32K_CLK] to valid value */
    bStatus = SCG_HWA_GetSirc32kValid();
    if (bStatus == true)
    {
        s_tClockSequenceInfo.tClockInfo[SCG_SIRC32K_CLK].eClkStatus = SCG_CLOCK_VALID;
        s_tClockSequenceInfo.tClockInfo[SCG_SIRC32K_CLK].u32Freq = SIRC32K_CLOCK;
    }
    else
    {
        s_tClockSequenceInfo.tClockInfo[SCG_SIRC32K_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_SIRC32K_CLK].u32Freq = 0U;
    }

}

static void SCG_SetFoscClockStatus(uint32_t u32FoscFreq)
{
    bool bStatus;
    uint32_t u32DivRegVal = 0U;

    /* check if FOSC is valid, then set s_tScgClockInfo[SCG_FOSC_CLK] to valid value */
    bStatus = SCG_HWA_GetFoscValid();
    if (bStatus == true)
    {
        s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].eClkStatus = SCG_CLOCK_VALID;
        s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq = u32FoscFreq;
        u32DivRegVal = SCG_HWA_GetFoscDiv();
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVH_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVH_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVH_CLK].u32Freq = SCG_CALCULATE_DIVH_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq, u32DivRegVal);

        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVM_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVM_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVM_CLK].u32Freq = SCG_CALCULATE_DIVM_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq, u32DivRegVal);

        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVL_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVL_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVL_CLK].u32Freq = SCG_CALCULATE_DIVL_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq, u32DivRegVal);
    }
    else
    {
        s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVH_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVH_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVM_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVM_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVL_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_FOSCDIVL_CLK].u32Freq = 0U;
    }
}

static void SCG_SetSoscClockStatus(uint32_t u32SoscFreq)
{
    bool bStatus;

    /* check if SOSC is valid, then set s_tScgClockInfo[SCG_SOSC_CLK] to valid value */
    bStatus = SCG_HWA_GetSoscValid();
    if (bStatus == true)
    {
        s_tClockSequenceInfo.tClockInfo[SCG_SOSC_CLK].eClkStatus = SCG_CLOCK_VALID;
        s_tClockSequenceInfo.tClockInfo[SCG_SOSC_CLK].u32Freq = u32SoscFreq;
    }
    else
    {
        s_tClockSequenceInfo.tClockInfo[SCG_SOSC_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_SOSC_CLK].u32Freq = 0U;
    }
}

static void SCG_SetPll0ClockStatus(void)
{
    uint8_t u8Temp, u8Mult, u8Prediv;
    uint32_t u32Temp = 0U;
    bool bStatus;
    uint32_t u32DivRegVal = 0U;

    /* check if PLL0 is valid, then set s_tScgClockInfo[SCG_PLL0_CLK] to valid value */
    bStatus = SCG_HWA_GetPll0Valid();
    if (bStatus == true)
    {
        /* if PLL0 clock source is FIRC, else PLL0 clock source is FOSC */
        u8Temp = SCG_HWA_GetPll0Src();
        if ((u8Temp == SCG_PLL0SOURCE_FIRC) && (s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].eClkStatus == SCG_CLOCK_VALID))
        {
            u32Temp = s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq >> 1U; /* Freq / 2*/
        }
        else if ((u8Temp == SCG_PLL0SOURCE_FOSC) && (s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].eClkStatus == SCG_CLOCK_VALID))
        {
            u32Temp = s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq >> 1U; /* Freq / 2*/
        }
        else
        {

        }

        u8Mult = SCG_HWA_GetPll0Mult();
        u8Prediv = SCG_HWA_GetPll0Prediv();
        u32Temp = u32Temp / (u8Prediv + 1U) * (u8Mult + 16U);
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].eClkStatus = SCG_CLOCK_VALID;
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].u32Freq = u32Temp;

        /* set pll0 DIV configuration information */
        u32DivRegVal = SCG_HWA_GetPll0Div();
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0DIVH_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVH_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0DIVH_CLK].u32Freq = SCG_CALCULATE_DIVH_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].u32Freq, u32DivRegVal);

        s_tClockSequenceInfo.tClockInfo[SCG_PLL0DIVM_CLK].eClkStatus = (SCG_StatusType)((uint8_t)SCG_CHECK_DIVM_EN(u32DivRegVal) ^ (uint8_t)1U);
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0DIVM_CLK].u32Freq = SCG_CALCULATE_DIVM_FREQ(s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].u32Freq, u32DivRegVal);
    }
    else
    {
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0DIVH_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0DIVH_CLK].u32Freq = 0U;
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0DIVM_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_PLL0DIVM_CLK].u32Freq = 0U;
    }
}

static void SCG_SetClockOutStatus(void)
{
    SCG_ClockoutSrcType eClockOutSrc;
    /* check clock out configuration */
    eClockOutSrc = (SCG_ClockoutSrcType)SCG_HWA_GetClkOutCfg();

    switch (eClockOutSrc)
    {
    case SCG_CLOCKOUT_SRC_OFF:
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].eClkStatus = SCG_CLOCK_DISABLE;
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].u32Freq = 0U;
        break;
    case SCG_CLOCKOUT_SRC_FOSC:
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].eClkStatus = s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].eClkStatus;
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq;
        break;
    case SCG_CLOCKOUT_SRC_SIRC:
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].eClkStatus = s_tClockSequenceInfo.tClockInfo[SCG_SIRC_CLK].eClkStatus;
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_SIRC_CLK].u32Freq;
        break;
    case SCG_CLOCKOUT_SRC_FIRC:
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].eClkStatus = s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].eClkStatus;
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq;
        break;
    case SCG_CLOCKOUT_SRC_SOSC:
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].eClkStatus = s_tClockSequenceInfo.tClockInfo[SCG_SOSC_CLK].eClkStatus;
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_SOSC_CLK].u32Freq;
        break;
    case SCG_CLOCKOUT_SRC_PLL0:
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].eClkStatus = s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].eClkStatus;
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].u32Freq;
        break;
    case SCG_CLOCKOUT_SRC_SIRC32K:
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].eClkStatus = s_tClockSequenceInfo.tClockInfo[SCG_SIRC32K_CLK].eClkStatus;
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_SIRC32K_CLK].u32Freq;
        break;
    default:
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].eClkStatus = SCG_CLOCK_UNDEFINE;
        s_tClockSequenceInfo.tClockInfo[SCG_SCG_CLKOUT_CLK].u32Freq = 0U;
        break;
    }

}

static void SCG_SetCoreClockStatus(void)
{
    uint8_t u8Temp, u8Div;
    uint32_t u32Temp = 0U;

    /* check core clock configuration */
    u8Temp = SCG_HWA_GetSysClkSrc();
    s_tClockSequenceInfo.tClockInfo[SCG_CORE_CLK].eClkStatus = SCG_CLOCK_VALID;
    s_tClockSequenceInfo.tClockInfo[SCG_CORE_CLK].u32Freq = UNKNOWN_CLOCK;
    s_tClockSequenceInfo.tClockInfo[SCG_BUS_CLK].eClkStatus = SCG_CLOCK_VALID;
    s_tClockSequenceInfo.tClockInfo[SCG_BUS_CLK].u32Freq = UNKNOWN_CLOCK;
    s_tClockSequenceInfo.tClockInfo[SCG_SLOW_CLK].eClkStatus = SCG_CLOCK_VALID;
    s_tClockSequenceInfo.tClockInfo[SCG_SLOW_CLK].u32Freq = UNKNOWN_CLOCK;
    if (u8Temp == SCG_CLOCK_SRC_FIRC)
    {
        if (s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].eClkStatus == SCG_CLOCK_VALID)
        {
            u8Div = SCG_HWA_GetSysClkDivCore();
            u32Temp = s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq / (u8Div + 1U);
            s_tClockSequenceInfo.tClockInfo[SCG_CORE_CLK].u32Freq = u32Temp;
            s_tClockSequenceInfo.eRunClock = SCG_RUNCLOCK_FIRC;
        }
    }
    else if (u8Temp == SCG_CLOCK_SRC_PLL0)
    {
        if ((s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].eClkStatus == SCG_CLOCK_VALID) &&
                (s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].u32Freq != UNKNOWN_CLOCK))
        {
            u8Div = SCG_HWA_GetSysClkDivCore();
            u32Temp = s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].u32Freq / (u8Div + 1U);
            s_tClockSequenceInfo.tClockInfo[SCG_CORE_CLK].u32Freq = u32Temp;
            u8Temp = SCG_HWA_GetPll0Src();
            if (u8Temp == SCG_PLL0SOURCE_FIRC)
            {
                s_tClockSequenceInfo.eRunClock = SCG_RUNCLOCK_PLL0_FIRC;
            }
            else
            {
                s_tClockSequenceInfo.eRunClock = SCG_RUNCLOCK_PLL0_FOSC;
            }
        }
    }
    else if (u8Temp == SCG_CLOCK_SRC_FOSC)
    {
        if ((s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].eClkStatus == SCG_CLOCK_VALID) &&
                (s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq != UNKNOWN_CLOCK))
        {
            u8Div = SCG_HWA_GetSysClkDivCore();
            u32Temp = s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq / (u8Div + 1U);
            s_tClockSequenceInfo.tClockInfo[SCG_CORE_CLK].u32Freq = u32Temp;
            s_tClockSequenceInfo.eRunClock = SCG_RUNCLOCK_FOSC;
        }
    }
    else
    {
        /* do nothing */
    }

    if (s_tClockSequenceInfo.tClockInfo[SCG_CORE_CLK].u32Freq != UNKNOWN_CLOCK)
    {
        u8Div = SCG_HWA_GetSysClkDivBus();
        u32Temp = s_tClockSequenceInfo.tClockInfo[SCG_CORE_CLK].u32Freq / (u8Div + 1U);
        s_tClockSequenceInfo.tClockInfo[SCG_BUS_CLK].u32Freq = u32Temp;

        u8Div = SCG_HWA_GetSysClkDivSlow();
        u32Temp = s_tClockSequenceInfo.tClockInfo[SCG_CORE_CLK].u32Freq / (u8Div + 1U);
        s_tClockSequenceInfo.tClockInfo[SCG_SLOW_CLK].u32Freq = u32Temp;
    }

}

/* ################################################################################## */
/* ################################ Global Functions ################################ */

/**
 * @brief Description SCG module maintains a list of SCG clock frequency and status.
 *  Which defined in global data instance as SCG_ClockInfoType s_tScgClockInfo[SCG_END_OF_CLOCKS],
 *  this function set all the clock source as SCG_CLOCK_UNDEFINE before clock set function called.
 */
void SCG_InitClockSrcStatus(void)
{
    if (s_tClockSequenceInfo.eRunClock == SCG_RUNCLOCK_NONE)
    {
        /* check if FIRC is valid, then set s_tScgClockInfo[SCG_FIRC_CLK] to valid value */
        SCG_SetFircClockStatus();

        /* check if SIRC is valid, then set s_tScgClockInfo[SCG_SIRC_CLK] to valid value */
        SCG_SetSircClockStatus();

        /* check if FOSC is valid, then set s_tScgClockInfo[SCG_FOSC_CLK] to valid value */
        SCG_SetFoscClockStatus((uint32_t)UNKNOWN_CLOCK);

        /* check if SIRC32K is valid, then set s_tScgClockInfo[SCG_SIRC32K_CLK] to valid value */
        SCG_SetSirc32kClockStatus();

        /* check if SOSC is valid, then set s_tScgClockInfo[SCG_SOSC_CLK] to valid value */
        SCG_SetSoscClockStatus((uint32_t)UNKNOWN_CLOCK);

        /* check if PLL0 is valid, then set s_tScgClockInfo[SCG_PLL0_CLK] to valid value */
        SCG_SetPll0ClockStatus();

        /* check clock out configuration */
        SCG_SetClockOutStatus();

        /* check core clock configuration */
        SCG_SetCoreClockStatus();
    }
}

/**
 * @brief Set SOSC configuration.
 * @param pSoscConfig: pointer to the soccType structure variable, which defined SOSC initial information.
 * @return true or false. SOSC would wait for SOSC valid in while loop within pre-dinfined limited time to check the SOSC valid or not.
 *         If SOSC still not valid(this may happen if external Slow OSC not placed),  it would return fail
 */
SCG_StatusType SCG_SetSOSC(SCG_SoscType *pSoscConfig)
{
    SCG_StatusType eStatus;
    uint32_t u32Temp;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();

    /*            Set SOSC             */
    if (pSoscConfig->bEnable == true)
    {
        /* configure recommend value */
        u32Temp = (uint32_t)(SCG_SOSCCFG_EOCV(64U) | SCG_SOSCCFG_GM_SEL(3U)  |
                             SCG_SOSCCFG_CURPRG_SF(3U) | SCG_SOSCCFG_CURPRG_COMP(3U));
        SCG_HWA_SetSoscCfg(u32Temp);

        u32Temp = SCG->SOSCCSR;
        /* unlock SOSC CSR register */
        u32Temp &= ~(uint32_t)SCG_SOSCCSR_LK_MASK;
        SCG_HWA_SetSoscCcr(u32Temp);

        /* Enable SOSC and configure bypass configuration */
        u32Temp &= ~(uint32_t)SCG_SOSCCSR_BYPASS_MASK;
        u32Temp |= SCG_SOSCCSR_EN(1U) | SCG_SOSCCSR_BYPASS(pSoscConfig->bBypass);
        SCG_HWA_SetSoscCcr(u32Temp);

        /*         Check SIRC valid         */
        u32Temp = SOSC_STABILIZATION_TIMEOUT;
        while ((SCG_HWA_GetSoscValid() == false) && (u32Temp > 0U))
        {
            u32Temp--;
        }

        if (u32Temp == 0U)
        {
            eStatus = SCG_CLOCK_TIMEOUT;
        }
        else
        {
            u32Temp = SCG->SOSCCSR;
            /* Configure CM CMRE and lock  */
            u32Temp |= SCG_SOSCCSR_CM(pSoscConfig->bCm);
            SCG_HWA_SetSoscCcr(u32Temp);

            u32Temp &= ~(uint32_t)SCG_SOSCCSR_CMRE_MASK;
            u32Temp |= SCG_SOSCCSR_CMRE(pSoscConfig->bCmre) |
                       SCG_SOSCCSR_LK(pSoscConfig->bLock);
            SCG_HWA_SetSoscCcr(u32Temp);

            eStatus = SCG_CLOCK_VALID;
        }
    }
    else
    {
        SCG_HWA_DisableSosc();
        eStatus = SCG_CLOCK_DISABLE;
        u32Temp = CLOCK_OFF_STABILIZATION_TIMEOUT;
        while ((SCG_HWA_GetSoscValid() == true) && (u32Temp > 0U))
        {
            u32Temp--;
        }
    }

    if((true == pSoscConfig->bCm) && (false == pSoscConfig->bCmre))
    {
        s_SoscClkErrNotify = pSoscConfig->pSoscClockErrorNotify;
    }


    /* set SOSC configuration information */
    SCG_SetSoscClockStatus(pSoscConfig->u32XtalFreq);

    return eStatus;
}

/**
 * \brief Set FOSC configuration
 *
 * \param pFoscConfig: pointer to the FOSCType structure data instance, which defined FOSC initial information.
 * \return  true or false. FOSCwould wait for FOSC valid in while loop within pre-dinfined limited time to check the FOSC valid or not.
 *          If FOSC still not valid(this may happen if external Fast OSC not placed),  it would return fail.
 */
SCG_StatusType SCG_SetFOSC(SCG_FoscType *pFoscConfig)
{
    SCG_StatusType eStatus;
    bool bComp_En = false;
    uint32_t u32Temp = 0U;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();

    if (pFoscConfig->bEnable == true)
    {
        if (pFoscConfig->bBypass == true)
        {
            bComp_En = false;
        }
        else
        {
            bComp_En = true;
        }

        /* COMP_EN is setting to 1  COMP_EN must be 1 when using an external crystal */
        /* Configure GM to the max value, GM_SEL: 15U */
        u32Temp = SCG_FOSCCFG_BYPASS(pFoscConfig->bBypass) | SCG_FOSCCFG_COMP_EN(bComp_En) |
                  SCG_FOSCCFG_EOCV(50U) | SCG_FOSCCFG_GM_SEL(15U) |
                  SCG_FOSCCFG_ALC_D(1U) | SCG_FOSCCFG_HYST_D(0U);
        SCG_HWA_SetFoscCfg(u32Temp);

        u32Temp = SCG->FOSCCSR;
        u32Temp &= ~(uint32_t)SCG_FOSCCSR_LK_MASK;
        SCG_HWA_SetFoscCsr(u32Temp);

        /* configure stop enable and enable FOSC */
        u32Temp &= ~(uint32_t)SCG_FOSCCSR_STEN_MASK;
        u32Temp |= SCG_FOSCCSR_EN(1U) | SCG_FOSCCSR_STEN(pFoscConfig->bSten);
        SCG_HWA_SetFoscCsr(u32Temp);

        /*               Check FOSC valid                       */

        u32Temp = FOSC_STABILIZATION_TIMEOUT;
        while ((SCG_HWA_GetFoscValid() == false) && (u32Temp > 0U))
        {
            u32Temp--;
        }

        if (u32Temp != 0U)
        {
            u32Temp = SCG->FOSCCSR;
            /* Configure CM CMRE and lock */
            u32Temp |= SCG_FOSCCSR_CM(pFoscConfig->bCm);
            SCG_HWA_SetFoscCsr(u32Temp);

            u32Temp &= ~(uint32_t)SCG_FOSCCSR_CMRE_MASK;
            u32Temp |= SCG_FOSCCSR_CMRE(pFoscConfig->bCmre) | SCG_FOSCCSR_LK(pFoscConfig->bLock);
            SCG_HWA_SetFoscCsr(u32Temp);

            eStatus = SCG_CLOCK_VALID;
        }
        else
        {
            eStatus = SCG_CLOCK_TIMEOUT;
        }

        if (eStatus == SCG_CLOCK_VALID)
        {
            /*
            	DIV setting process:
            	MCU_FC4150_512K:   Clear FOSCDIV[DIVH_EN] --> Configure FOSCDIV[DIVH] --> Set FOSCDIV[DIVH_EN]
            	MCU_FC4150_2M:    Clear FOSCDIV[DIVH_EN], wait FOSCDIV[DIVH_ACK] clear
            					--> Configure FIRCDIV[DIVH]
            					--> Set FOSCDIV[DIVH_EN], wait FOSCDIV[DIVH_ACK] is set
            */
            SCG_HWA_DiableFoscDiv();
            u32Temp = CLOCK_DIV_STABILIZATION_TIMEOUT;
            while (((SCG->FOSCDIV & (SCG_FOSCDIV_DIVH_ACK_MASK | SCG_FOSCDIV_DIVM_ACK_MASK | SCG_FOSCDIV_DIVL_ACK_MASK)) != 0U)
                    && (u32Temp > 0U))
            {
                u32Temp--;
            }
            u32Temp = SCG->FOSCDIV;
            u32Temp &= ~(uint32_t)(SCG_FOSCDIV_DIVL_MASK | SCG_FOSCDIV_DIVM_MASK | SCG_FOSCDIV_DIVH_MASK);
            u32Temp |= ((((uint32_t)pFoscConfig->eDivH << SCG_FOSCDIV_DIVH_SHIFT) & SCG_FOSCDIV_DIVH_MASK) |
                        (((uint32_t)pFoscConfig->eDivM << SCG_FOSCDIV_DIVM_SHIFT) & SCG_FOSCDIV_DIVM_MASK) |
                        (((uint32_t)pFoscConfig->eDivL << SCG_FOSCDIV_DIVL_SHIFT) & SCG_FOSCDIV_DIVL_MASK));
            SCG_HWA_SetFoscDiv(u32Temp) ;

            SCG_HWA_EnableFoscDiv();
            u32Temp = CLOCK_DIV_STABILIZATION_TIMEOUT;
            while ((((SCG->FOSCDIV & (uint32_t)SCG_FOSCDIV_DIVL_ACK_MASK) == 0U) ||
                    ((SCG->FOSCDIV & (uint32_t)SCG_FOSCDIV_DIVM_ACK_MASK) == 0U) ||
                    ((SCG->FOSCDIV & (uint32_t)SCG_FOSCDIV_DIVH_ACK_MASK) == 0U)) &&
                    (u32Temp > 0U))
            {
                u32Temp--;
            }
        }
    }
    else
    {
        /* if core clock source if from fosc, must switch core clock source first */
        if ((s_tClockSequenceInfo.eRunClock == SCG_RUNCLOCK_FOSC) ||
                (s_tClockSequenceInfo.eRunClock == SCG_RUNCLOCK_PLL0_FOSC))
        {
            eStatus = SCG_CLOCK_SEQUENCE_ERROR;
        }
        else
        {
            SCG_HWA_DisableFosc();
            eStatus = SCG_CLOCK_DISABLE;
            u32Temp = CLOCK_OFF_STABILIZATION_TIMEOUT;
            while ((SCG_HWA_GetFoscValid() == true) && (u32Temp > 0U))
            {
                u32Temp--;
            }
        }
    }

    if((true == pFoscConfig->bCm) && (false == pFoscConfig->bCmre))
    {
        s_FoscClkErrNotify = pFoscConfig->pFoscClockErrorNotify;
    }

    /* set FOSC configuration information */
    SCG_SetFoscClockStatus(pFoscConfig->u32XtalFreq);

    return eStatus;
}

/**
 * \brief Set SIRC configuration.
 * \param pSircConfig: pointer to the SIRCType structure data instance, which defined SIRC initial information
 * \return true or false. SIRCwould wait for SIRC valid in while loop within pre-dinfined limited time
 *         to check the SIRC valid or not. If SOSC still not valid,  it would return fail.
 */
SCG_StatusType SCG_SetSIRC(SCG_SircType *pSircConfig)
{
    SCG_StatusType eStatus;
    uint32_t u32Temp;
    uint16_t u16TrimDiv;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();

    u32Temp = SCG->SIRCCSR;
    u32Temp &= ~(uint32_t)(SCG_SIRCCSR_CM_MASK | SCG_SIRCCSR_LK_MASK | SCG_SIRCCSR_TRUP_MASK |
                           SCG_SIRCCSR_TREN_MASK | SCG_SIRCCSR_LPEN_MASK |
                           SCG_SIRCCSR_STEN_MASK);
    u32Temp |= (uint32_t)(SCG_SIRCCSR_CM(pSircConfig->bCm) | SCG_SIRCCSR_LK(pSircConfig->bLock) |
                          SCG_SIRCCSR_TRUP(pSircConfig->bTrEn) |
                          SCG_SIRCCSR_TREN(pSircConfig->bTrEn) |
                          SCG_SIRCCSR_LPEN(pSircConfig->bLpen) |
                          SCG_SIRCCSR_STEN(pSircConfig->bSten));
    SCG_HWA_SetSircCsr(u32Temp);

    /*               Check SIRC valid                       */
    u32Temp = SIRC_STABILIZATION_TIMEOUT;
    while ((SCG_HWA_GetSircValid() == false) && (u32Temp > 0U))
    {
        u32Temp--;
    }

    if (u32Temp != 0U)
    {
        eStatus = SCG_CLOCK_VALID;
    }
    else
    {
        eStatus = SCG_CLOCK_TIMEOUT;
    }

    if (eStatus == SCG_CLOCK_VALID)
    {
        /*
            DIV setting process:
            MCU_FC4150_512K:   Clear SIRCDIV[DIVH_EN] --> Configure SIRCDIV[DIVH] --> Set SIRCDIV[DIVH_EN]
            MCU_FC4150_2M:    Clear SIRCDIV[DIVH_EN], wait SIRCDIV[DIVH_ACK] clear
                            --> Configure FIRCDIV[DIVH]
                            --> Set SIRCDIV[DIVH_EN], wait SIRCDIV[DIVH_ACK] is set
        */
        SCG_HWA_DiableSircDiv();
        u32Temp = CLOCK_DIV_STABILIZATION_TIMEOUT;
        while (((SCG->SIRCDIV & (SCG_SIRCDIV_DIVH_MASK | SCG_SIRCDIV_DIVM_MASK | SCG_SIRCDIV_DIVL_MASK)) != 0U)
                && (u32Temp > 0U))
        {
            u32Temp--;
        }
        u32Temp = SCG->SIRCDIV;
        u32Temp &= ~(uint32_t)(SCG_SIRCDIV_DIVL_MASK | SCG_SIRCDIV_DIVM_MASK | SCG_SIRCDIV_DIVH_MASK);
        u32Temp |= (uint32_t)((((uint32_t)pSircConfig->eDivH << SCG_SIRCDIV_DIVH_SHIFT) & SCG_SIRCDIV_DIVH_MASK) |
                              (((uint32_t)pSircConfig->eDivM << SCG_SIRCDIV_DIVM_SHIFT) & SCG_SIRCDIV_DIVM_MASK) |
                              (((uint32_t)pSircConfig->eDivL << SCG_SIRCDIV_DIVL_SHIFT) & SCG_SIRCDIV_DIVL_MASK));
        SCG_HWA_SetSircDiv(u32Temp);
        SCG_HWA_EnableSircDiv();
        u32Temp = CLOCK_DIV_STABILIZATION_TIMEOUT;
        while ((((SCG->SIRCDIV & (uint32_t)SCG_SIRCDIV_DIVL_ACK_MASK) == 0U) ||
                ((SCG->SIRCDIV & (uint32_t)SCG_SIRCDIV_DIVM_ACK_MASK) == 0U) ||
                ((SCG->SIRCDIV & (uint32_t)SCG_SIRCDIV_DIVH_ACK_MASK) == 0U)) &&
                (u32Temp > 0U))
        {
            u32Temp--;
        }

        /*   SIRC  configuration SIRCTCFG    */
        if (pSircConfig->bTrEn == false)
        {
            /*   Trim enalbe disabled, just using IC internal IRC trim value */
        }
        else
        {
            /*   set SIRCTCFG trim configuration    */
            if (pSircConfig->u8TrimSrc == SCG_IRC_TRIMSRC_FOSC)
            {
                /*   Trim clock source choosed FOSC   */
                u16TrimDiv = (uint16_t)(s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq / 1000000U - 1U);
            }
            else if (pSircConfig->u8TrimSrc == SCG_IRC_TRIMSRC_SOSC)
            {
                /*   Trim clock source choosed SOSC   */
                u16TrimDiv = 0U;
            }
            else
            {
                u16TrimDiv = 0U;
                /*   do nothing   */
            }
            /* Setting the SAMPLE bit and enabling DELAY means more sampling time and longer calibration time,
             * which leads to more accurate calibration. */
            u32Temp = (uint32_t)(SCG_SIRCTCFG_TRIMSRC(pSircConfig->u8TrimSrc) |
                                 SCG_SIRCTCFG_TRIMDIV(u16TrimDiv) |
                                 SCG_SIRCTCFG_SAMPLE_MASK |
                                 SCG_SIRCTCFG_DELAY_MASK);
            SCG_HWA_SetSircTcfg(u32Temp);
        }

    }

    if(true == pSircConfig->bCm)
    {
        s_SircClkErrNotify = pSircConfig->pSircClockErrorNotify;
    }

    /* set SIRC configuration information */
    SCG_SetSircClockStatus();

    return eStatus;
}

/**
 * \brief Set SIRC32K configuration.
 * \param pSirc32kConfig: pointer to the Sirc32kType structure data instance, which defined SIRC32K initial information
 * \return true or false.
 */
SCG_StatusType SCG_SetSIRC32K(SCG_Sirc32kType *pSirc32kConfig)
{
    SCG_StatusType eStatus;
    uint32_t u32Temp = 0U;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();

    u32Temp = (uint32_t)(SCG_SIRC32KCSR_LK(pSirc32kConfig->bLock) | SCG_SIRC32KCSR_EN(pSirc32kConfig->bEn));
    SCG_HWA_SetSirc32kCsr(u32Temp);

    /*               Check SIRC valid                       */
    u32Temp = SIRC_STABILIZATION_TIMEOUT;
    while ((SCG_HWA_GetSirc32kValid() == false) && (u32Temp > 0U))
    {
        u32Temp--;
    }

    if (u32Temp == 0U)
    {
        eStatus = SCG_CLOCK_TIMEOUT;
    }
    else
    {
        eStatus = SCG_CLOCK_VALID;
    }

    /* set SIRC32K configuration information */
    SCG_SetSirc32kClockStatus();

    return eStatus;
}

/**
 * \brief Set FIRC configuration.
 * \param pFircConfig: pointer to the FIRCType structure data instance, which defined FIRC initial information.
 * \return true or false. FIRCwould wait for FIRC valid in while loop within pre-dinfined limited time
 *         to check the FIRC valid or not. If FIRC still not valid,  it would return fail.
 */
SCG_StatusType SCG_SetFIRC(SCG_FircType *pFircConfig)
{
    SCG_StatusType eStatus;
    uint32_t u32Temp;
    uint16_t u16TrimDiv;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();

    /*            Set FIRCCSR           */
    if (pFircConfig->bEnable == true)
    {
        SCG_HWA_SetFircCfg(SCG_FIRCCFG_CLKEN(3U));

        u32Temp = SCG->FIRCCSR;
        u32Temp &= ~(uint32_t)(SCG_FIRCCSR_TRUP_MASK | SCG_FIRCCSR_TREN_MASK | SCG_FIRCCSR_STEN_MASK);
        u32Temp |= (uint32_t)(SCG_FIRCCSR_TRUP(pFircConfig->bTrEn) |    /*    configure TRUP and EN together with TREN setting   */
                              SCG_FIRCCSR_TREN(pFircConfig->bTrEn) |
                              SCG_FIRCCSR_STEN(pFircConfig->bSten) |
                              SCG_FIRCCSR_EN(1U));
        SCG_HWA_SetFircCsr(u32Temp);

        u32Temp = FIRC_STABILIZATION_TIMEOUT;
        /*               Check FIRC valid                       */
        while ((SCG_HWA_GetFircValid() == false) && (u32Temp > 0U))
        {
            u32Temp--;
        }

        if (u32Temp != 0U)
        {
            /* Configure CM and lock */
            u32Temp = SCG->FIRCCSR;
            u32Temp |= (uint32_t)SCG_FIRCCSR_CM(pFircConfig->bCm);
            SCG_HWA_SetFircCsr(u32Temp);
            u32Temp |= (uint32_t)SCG_FIRCCSR_LK(pFircConfig->bLock);
            SCG_HWA_SetFircCsr(u32Temp);

            eStatus = SCG_CLOCK_VALID;
        }
        else
        {
            eStatus = SCG_CLOCK_TIMEOUT;
        }

        if (eStatus == SCG_CLOCK_VALID)
        {
            /*               Set FIRCDIV                       */
            /*
            	DIV setting process:
            	MCU_FC4150_512K:   Clear FIRCDIV[DIVH_EN] --> Configure FIRCDIV[DIVH] --> Set FIRCDIV[DIVH_EN]
            	MCU_FC4150_2M:    Clear FIRCDIV[DIVH_EN], wait FIRCDIV[DIVH_ACK] clear
            					--> Configure FIRCDIV[DIVH]
            					--> Set FIRCDIV[DIVH_EN], wait FIRCDIV[DIVH_ACK] is set
            */
            SCG_HWA_DiableFircDiv();
            u32Temp = CLOCK_DIV_STABILIZATION_TIMEOUT;
            while (((SCG->FIRCDIV & (SCG_FIRCDIV_DIVH_ACK_MASK | SCG_FIRCDIV_DIVM_ACK_MASK | SCG_FIRCDIV_DIVL_ACK_MASK)) != 0U) &&
                    ((u32Temp > 0U)))
            {
                u32Temp--;
            }
            u32Temp = SCG->FIRCDIV;
            u32Temp &= ~(uint32_t)(SCG_FIRCDIV_DIVL_MASK | SCG_FIRCDIV_DIVM_MASK | SCG_FIRCDIV_DIVH_MASK);
            u32Temp |= (uint32_t)((((uint32_t)pFircConfig->eDivH << SCG_FIRCDIV_DIVH_SHIFT) & SCG_FIRCDIV_DIVH_MASK) |
                                  (((uint32_t)pFircConfig->eDivM << SCG_FIRCDIV_DIVM_SHIFT) & SCG_FIRCDIV_DIVM_MASK) |
                                  (((uint32_t)pFircConfig->eDivL << SCG_FIRCDIV_DIVL_SHIFT) & SCG_FIRCDIV_DIVL_MASK));

            SCG_HWA_SetFircDiv(u32Temp);
            SCG_HWA_EnableFircDiv();
            u32Temp = CLOCK_DIV_STABILIZATION_TIMEOUT;
            while ((((SCG->FIRCDIV & (uint32_t)SCG_FIRCDIV_DIVL_ACK_MASK) == 0U) ||
                    ((SCG->FIRCDIV & (uint32_t)SCG_FIRCDIV_DIVM_ACK_MASK) == 0U) ||
                    ((SCG->FIRCDIV & (uint32_t)SCG_FIRCDIV_DIVH_ACK_MASK) == 0U)) &&
                    (u32Temp > 0U))
            {
                u32Temp--;
            }

            /*   For clock autotrim, set TREN to True together with TRUP to True   */
            if (pFircConfig->bTrEn == true)
            {
                /*   set FIRCTCFG trim configuration    */
                if (pFircConfig->u8TrimSrc == SCG_IRC_TRIMSRC_FOSC)
                {
                    /*   Trim clock source choose FOSC   */
                    u16TrimDiv = (uint16_t)(s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq / 1000000U - 1U);
                }
                else if (pFircConfig->u8TrimSrc == SCG_IRC_TRIMSRC_SOSC)
                {
                    /*   Trim clock source choose SOSC   */
                    u16TrimDiv = 0U;
                }
                else
                {
                    /*   do nothing   */
                    u16TrimDiv = 0U;
                }
                /* Setting the SAMPLE bit and enabling DELAY means more sampling time and longer calibration time,
                 * which leads to more accurate calibration. */
                u32Temp = (uint32_t)(SCG_FIRCTCFG_TRIMSRC(pFircConfig->u8TrimSrc) |
                                     SCG_FIRCTCFG_TRIMDIV(u16TrimDiv) |
                                     SCG_FIRCTCFG_SAMPLE_MASK |
                                     SCG_FIRCTCFG_DELAY_MASK);
                SCG_HWA_SetFircTcfg(u32Temp);
            }
            else
            {
                /*   Trim disabled, just using IC internal IRC trim value */
            }

        }
        else
        {
            SCG_HWA_DisableFirc();
        }
    }
    else
    {
        /* if core clock source if from fosc, must switch core clock source first */
        if ((s_tClockSequenceInfo.eRunClock == SCG_RUNCLOCK_FIRC) ||
                (s_tClockSequenceInfo.eRunClock == SCG_RUNCLOCK_PLL0_FIRC))
        {
            eStatus = SCG_CLOCK_SEQUENCE_ERROR;
        }
        else
        {
            eStatus = SCG_CLOCK_DISABLE;
            SCG_HWA_DisableFirc();

            u32Temp = CLOCK_OFF_STABILIZATION_TIMEOUT;
            while ((SCG_HWA_GetFircValid() == true) && (u32Temp > 0U))
            {
                u32Temp--;
            }
        }
    }

    if(true == pFircConfig->bCm)
    {
        s_FircClkErrNotify = pFircConfig->pFircClockErrorNotify;
    }


    SCG_SetFircClockStatus();

    return eStatus;
}

/**
 * \brief Set PLL0 configuration.
 * \param pPll0Config: pointer to the PLL0Type structure data instance, which defined PLL0 initial information.
 * \return true or false. PLL0 would wait for PLL0 valid in while loop within pre-dinfined limited time
 *         to check the PLL0 valid or not. If PLL0 still not valid(this may happen if external Fast OSC not placed),
 *         it would return fail. PLL0 would check the status of input clock source(FIRC or FOSC), if it is not valid, it would return false.
 */
SCG_StatusType SCG_SetPLL0(SCG_Pll0Type *pPll0Config)
{
    SCG_StatusType eStatus;
    uint32_t u32Freq;
    uint32_t u32Temp;
    uint32_t u32Index;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();


    /* if core clock source if from fosc, must switch core clock source first */
    if ((s_tClockSequenceInfo.eRunClock == SCG_RUNCLOCK_PLL0_FIRC) ||
            (s_tClockSequenceInfo.eRunClock == SCG_RUNCLOCK_PLL0_FOSC))
    {
        eStatus = SCG_CLOCK_SEQUENCE_ERROR;
    }
    else
    {
        if (true == pPll0Config->bEnable)
        {
            switch (pPll0Config->eSrc)
            {
            case SCG_PLL0SOURCE_FOSC:
            {
                if (s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].eClkStatus == SCG_CLOCK_VALID)
                {
                    u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq;
                    u32Freq = u32Freq / (pPll0Config->ePrediv + 1U)  * (pPll0Config->eMult + 16U);
                    u32Freq = u32Freq / 2U;
                    if ((u32Freq >= PLL0_CLK_MAX) || (u32Freq <= PLL0_CLK_MIN))
                    {
                        eStatus = SCG_CLOCK_PARAM_INVALID;
                    }
                    else
                    {
                        eStatus = SCG_CLOCK_VALID;
                    }
                }
                else
                {
                    eStatus = SCG_CLOCK_ERROR;
                }
            }
            break;

            case SCG_PLL0SOURCE_FIRC:
            {
                if (s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].eClkStatus == SCG_CLOCK_VALID)
                {
                    /*   PLL0 input is FIRC clock/2      */
                    u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq / 2U;
                    u32Freq = u32Freq / (pPll0Config->ePrediv + 1U)  * (pPll0Config->eMult + 16U);
                    u32Freq = u32Freq / 2U;
                    if ((u32Freq > PLL0_CLK_MAX) || (u32Freq < PLL0_CLK_MIN))
                    {
                        eStatus = SCG_CLOCK_PARAM_INVALID;
                    }
                    else
                    {
                        eStatus = SCG_CLOCK_VALID;
                    }
                }
                else
                {
                    eStatus = SCG_CLOCK_ERROR;
                }
            }
            break;

            default:
                eStatus = SCG_CLOCK_ERROR;
                break;
            }

            if (eStatus == SCG_CLOCK_VALID)
            {
                if (SCG_PLL0PREDIV_BY2 == pPll0Config->ePrediv)
                {
                    u32Temp =  SCG_PLL0CFG_PREDIV(SCG_PLL0PREDIV_BY4) | SCG_PLL0CFG_MULT(pPll0Config->eMult) |
                               SCG_PLL0CFG_SOURCE(pPll0Config->eSrc) ;
                    SCG_HWA_SetPll0Cfg(u32Temp);
                    u32Temp = SCG_PLL0CSR_EN_MASK;
                    SCG_HWA_SetPll0Csr(u32Temp);
                    for (u32Index = 0; u32Index < 200; u32Index++)
                    {
                    	__asm("nop");
                    }

                    SCG_HWA_SetPll0Csr(0U);
                    u32Temp =  SCG_PLL0CFG_PREDIV(pPll0Config->ePrediv) | SCG_PLL0CFG_MULT(pPll0Config->eMult) |
                               SCG_PLL0CFG_SOURCE(pPll0Config->eSrc) ;
                    SCG_HWA_SetPll0Cfg(u32Temp);
                }
                else
                {
                    u32Temp =  SCG_PLL0CFG_PREDIV(pPll0Config->ePrediv) | SCG_PLL0CFG_MULT(pPll0Config->eMult) |
                               SCG_PLL0CFG_SOURCE(pPll0Config->eSrc) ;
                    SCG_HWA_SetPll0Cfg(u32Temp);
                }

                u32Temp = SCG_PLL0CSR_EN(pPll0Config->bEnable)  | SCG_PLL0CSR_STEN(pPll0Config->bSten);
                SCG_HWA_SetPll0Csr(u32Temp);

                /*               Wait till PLL0 valid                       */
                u32Temp = PLL0_STABILIZATION_TIMEOUT;
                while ((SCG_HWA_GetPll0Locked() == false) && (u32Temp > 0U))
                {
                    u32Temp--;
                }

                if (u32Temp != 0U)
                {
                    u32Temp = SCG->PLL0CSR;
                    /* Configure CM CMRE and lock */
                    u32Temp |= SCG_PLL0CSR_CM(pPll0Config->bCm);
                    SCG_HWA_SetPll0Csr(u32Temp);
                    u32Temp |= SCG_PLL0CSR_CMRE(pPll0Config->bCmre) | SCG_PLL0CSR_LK(pPll0Config->bLock);
                    SCG_HWA_SetPll0Csr(u32Temp);

                    eStatus = SCG_CLOCK_VALID;
                }
                else
                {
                    eStatus = SCG_CLOCK_TIMEOUT;
                }

                if (eStatus == SCG_CLOCK_VALID)
                {
                    /*
                        DIV setting process:
                        MCU_FC4150_512K:   Clear PLL0DIV[DIVH_EN] --> Configure PLL0DIV[DIVH] --> Set PLL0DIV[DIVH_EN]
                        MCU_FC4150_2M:    Clear PLL0DIV[DIVH_EN], wait PLL0DIV[DIVH_ACK] clear
                                        --> Configure FIRCDIV[DIVH]
                                        --> Set PLL0DIV[DIVH_EN], wait PLL0DIV[DIVH_ACK] is set
                    */
                    SCG_HWA_DiablePll0Div();
                    u32Temp = CLOCK_DIV_STABILIZATION_TIMEOUT;
                    while (((SCG->PLL0DIV & (SCG_PLL0DIV_DIVL_ACK_MASK | SCG_PLL0DIV_DIVM_ACK_MASK | SCG_PLL0DIV_DIVH_ACK_MASK)) != 0U) &&
                            (u32Temp > 0U))
                    {
                        u32Temp--;
                    }
                    u32Temp = SCG->PLL0DIV;
                    u32Temp &= ~(uint32_t)(SCG_PLL0DIV_DIVL_MASK | SCG_PLL0DIV_DIVM_MASK | SCG_PLL0DIV_DIVH_MASK);
                    u32Temp = (uint32_t)((((uint32_t)pPll0Config->eDivH << SCG_PLL0DIV_DIVH_SHIFT) & SCG_PLL0DIV_DIVH_MASK) |
                                         (((uint32_t)pPll0Config->eDivM << SCG_PLL0DIV_DIVM_SHIFT) & SCG_PLL0DIV_DIVM_MASK) |
                                         (((uint32_t)pPll0Config->eDivL << SCG_PLL0DIV_DIVL_SHIFT) & SCG_PLL0DIV_DIVL_MASK));
                    SCG_HWA_SetPll0Div(u32Temp);

                    SCG_HWA_EnablePll0Div();
                    u32Temp = CLOCK_DIV_STABILIZATION_TIMEOUT;
                    while ((((SCG->PLL0DIV & (uint32_t)SCG_PLL0DIV_DIVL_ACK_MASK) == 0U) ||
                            ((SCG->PLL0DIV & (uint32_t)SCG_PLL0DIV_DIVM_ACK_MASK) == 0U) ||
                            ((SCG->PLL0DIV & (uint32_t)SCG_PLL0DIV_DIVH_ACK_MASK) == 0U)) &&
                            (u32Temp > 0U))
                    {
                        u32Temp--;
                    }

                }
            }
        }
        else
        {
            eStatus = SCG_CLOCK_DISABLE;
            /* Unlock PLL0 */
            SCG_HWA_UnlockPll0();
            /* Disable PLL0 */
            SCG_HWA_DisablePll0();

            u32Temp = CLOCK_OFF_STABILIZATION_TIMEOUT;
            while ((SCG_HWA_GetPll0Locked() == true) && (u32Temp > 0U))
            {
                u32Temp--;
            }
        }

        if((true == pPll0Config->bCm) && (false == pPll0Config->bCmre))
        {
            s_Pll0ClkErrNotify = pPll0Config->pPll0ClockErrorNotify;
        }
    }

    /* set PLL0 configuration information */
    SCG_SetPll0ClockStatus();
    return eStatus;
}

/**
 * \brief Set system run time clock and related CORE/BUS/SLOW clock.
 * \param pSysClkConfig: pointer to the clockCtrlType structure data instance,
 *        which defined for system clock selection.
 * \return true or false. This function would check the clock source status before set it system clock,
 *         if the chosen clock source is invalid, it would return false.
 */
SCG_StatusType SCG_SetClkCtrl(SCG_ClockCtrlType *pSysClkConfig)
{
    SCG_StatusType eStatus;
    uint32_t u32Temp, u32Freq, u32FreqCore, u32FreqBus, u32FreqSlow;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();

    switch (pSysClkConfig->eSrc)
    {
    case SCG_CLOCK_SRC_FOSC:
    {
        if (s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].eClkStatus == SCG_CLOCK_VALID)
        {
            /*   PLL0 input is FIRC clock/2      */
            u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].u32Freq;
            u32FreqCore =  u32Freq / (pSysClkConfig->eDivCore + 1U);
            u32FreqBus =  u32FreqCore / (pSysClkConfig->eDivBus + 1U);
            u32FreqSlow =  u32FreqCore / (pSysClkConfig->eDivSlow + 1U);
            if ((u32FreqCore > SYS_CORE_CLK_MAX) || (u32FreqBus > SYS_BUS_CLK_MAX) || (u32FreqSlow > SYS_SLOW_CLK_MAX))
            {
                eStatus = SCG_CLOCK_PARAM_INVALID;
            }
            else
            {
                eStatus = SCG_CLOCK_VALID;
            }
        }
        else
        {
            eStatus = SCG_CLOCK_ERROR;
        }
    }
    break;

    case SCG_CLOCK_SRC_FIRC:
    {
        if (s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].eClkStatus == SCG_CLOCK_VALID)
        {
            /*   PLL0 input is FIRC clock/2      */
            u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].u32Freq;
            u32FreqCore =  u32Freq / (pSysClkConfig->eDivCore + 1U);
            u32FreqBus =  u32FreqCore / (pSysClkConfig->eDivBus + 1U);
            u32FreqSlow =  u32FreqCore / (pSysClkConfig->eDivSlow + 1U);
            if ((u32FreqCore > SYS_CORE_CLK_MAX) || (u32FreqBus > SYS_BUS_CLK_MAX) || (u32FreqSlow > SYS_SLOW_CLK_MAX))
            {
                eStatus = SCG_CLOCK_PARAM_INVALID;
            }
            else
            {
                eStatus = SCG_CLOCK_VALID;
            }
        }
        else
        {
            eStatus = SCG_CLOCK_ERROR;
        }
    }
    break;

    case SCG_CLOCK_SRC_PLL0:
    {
        if (s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].eClkStatus == SCG_CLOCK_VALID)
        {
            /*   PLL0 input is FIRC clock/2      */
            u32Freq = s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].u32Freq;
            u32FreqCore = u32Freq / (pSysClkConfig->eDivCore + 1U);
            u32FreqBus = u32FreqCore / (pSysClkConfig->eDivBus + 1U);
            u32FreqSlow = u32FreqCore / (pSysClkConfig->eDivSlow + 1U);
            if ((u32FreqCore > SYS_CORE_CLK_MAX) || (u32FreqBus > SYS_BUS_CLK_MAX) || (u32FreqSlow > SYS_SLOW_CLK_MAX))
            {
                eStatus = SCG_CLOCK_PARAM_INVALID;
            }
            else
            {
                eStatus = SCG_CLOCK_VALID;
            }
        }
        else
        {
            eStatus = SCG_CLOCK_ERROR;
        }
    }
    break;

    default:
        eStatus = SCG_CLOCK_ERROR;
        break;
    }

    if (eStatus == SCG_CLOCK_VALID)
    {
        u32Temp = (uint32_t)((uint32_t)SCG_CCR_SYSCLK_CME(pSysClkConfig->bSysClkMonitor) |
                             (uint32_t)SCG_CCR_SCS(pSysClkConfig->eSrc) |
                             (uint32_t)SCG_CCR_DIVCORE(pSysClkConfig->eDivCore) |
                             (uint32_t)SCG_CCR_DIVBUS(pSysClkConfig->eDivBus)  |
                             (uint32_t)SCG_CCR_DIVSLOW(pSysClkConfig->eDivSlow));
        SCG_HWA_SetCCR(u32Temp);

        u32Temp = SCG_CLKSRC_STABILIZATION_TIMEOUT;
        while ((SCG_HWA_GetSysClkUPRD() == false) && (u32Temp > 0U))
        {
            u32Temp--;
        }

        /*   Time out, clock select failed     */
        if (u32Temp == 0U)
        {
            eStatus = SCG_CLOCK_TIMEOUT;
        }
        else if (SCG_HWA_GetSysClkSrc() != pSysClkConfig->eSrc)
        {
            eStatus = SCG_CLOCK_ERROR;

        }
        else
        {
        }
    }

    /* set core clock configuration information */
    SCG_SetCoreClockStatus();

    return eStatus;
}

/**
 * \brief  Switch system clock source during run time.
 * \param eClock: target clock source user want to switch.
 * \return true or false. This function check if the clock source is in proper range.
 */
SCG_StatusType SCG_SwitchClkCtrlSrc(SCG_ClockSrcType eClock)
{
    SCG_StatusType eStatus = SCG_CLOCK_ERROR;
    uint32_t u32Temp;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();

    switch (eClock)
    {
    case SCG_CLOCK_SRC_FOSC:
    {
        if (s_tClockSequenceInfo.tClockInfo[SCG_FOSC_CLK].eClkStatus == SCG_CLOCK_VALID)
        {
            u32Temp = SCG->CCR;
            u32Temp &= ~SCG_CCR_SCS_MASK;
            u32Temp |= SCG_CCR_SCS(eClock);
            SCG_HWA_SetCCR(u32Temp);
            eStatus = SCG_CLOCK_VALID;
        }
    }
    break;

    case SCG_CLOCK_SRC_FIRC:
    {
        if (s_tClockSequenceInfo.tClockInfo[SCG_FIRC_CLK].eClkStatus == SCG_CLOCK_VALID)
        {
            u32Temp = SCG->CCR;
            u32Temp &= ~SCG_CCR_SCS_MASK;
            u32Temp |= SCG_CCR_SCS(eClock);
            SCG_HWA_SetCCR(u32Temp);
            eStatus = SCG_CLOCK_VALID;
        }
    }
    break;

    case SCG_CLOCK_SRC_PLL0:
    {
        if (s_tClockSequenceInfo.tClockInfo[SCG_PLL0_CLK].eClkStatus == SCG_CLOCK_VALID)
        {
            u32Temp = SCG->CCR;
            u32Temp &= ~SCG_CCR_SCS_MASK;
            u32Temp |= SCG_CCR_SCS(eClock);
            SCG_HWA_SetCCR(u32Temp);
            eStatus = SCG_CLOCK_VALID;
        }
    }
    break;

    default:
        /* do nothing */
        break;
    }

    if (eStatus == SCG_CLOCK_VALID)
    {
        /* set core clock configuration information */
        SCG_SetCoreClockStatus();
    }
    return eStatus;
}

/**
 * \brief Report the clock source status and frequency configured in MCU run time.
 *        The clock frequency and status would change by clock set function.
 * \param eScgClockName: the clock source to query
 * \param pFrequency: frequency variable point to get the frequency value
 * \return true or false. This indicate the clock source status invalid or request clock source out of
 *         range.
 */
SCG_StatusType SCG_GetScgClockFreq(SCG_ClkSrcType eScgClockName,  uint32_t *pFrequency)
{
    SCG_StatusType eStatus;
    if (eScgClockName > SCG_END_OF_CLOCKS)
    {
        eStatus = SCG_CLOCK_PARAM_INVALID;
        *pFrequency = 0U;
    }
    else
    {
        eStatus = s_tClockSequenceInfo.tClockInfo[eScgClockName].eClkStatus;
        *pFrequency = s_tClockSequenceInfo.tClockInfo[eScgClockName].u32Freq;
    }
    return eStatus;
}

/**
 * \brief  Set clock out source in SCG. it set SCG_CLKOUTCFG [CLKOUTSEL].
 * \param pSysClkConfig: pointer to the clockCtrlType structure data instance,
 *        which defined for system clock selection.
 */
void SCG_SetClkOut(SCG_ClockCtrlType *pSysClkConfig)
{
    uint32_t u32Temp;

    /* if run clock is none, the function will init s_tClockSequenceInfo buffer first */
    SCG_InitClockSrcStatus();

    SCG_ClockoutSrcType eClockOutSrc = SCG_CLOCKOUT_SRC_PLL0;
    SCG_NvmClkSrcType eNvmClockSrc = SCG_NVMCLK_SRC_FIRC;
    eClockOutSrc = pSysClkConfig->eClkOutSrc;
    eNvmClockSrc = pSysClkConfig->eNvmClkSrc;

    /*   disable as NVMCLK source first time
         set clock out source together            */
    u32Temp = SCG->CLKOUTCFG;
    u32Temp &= ~(SCG_CLKOUTCFG_CLKOUTSEL_MASK | SCG_CLKOUTCFG_NVMCLK_FIRC_MASK | SCG_CLKOUTCFG_NVMCLK_SIRC_MASK);
    u32Temp |= (uint32_t)(SCG_CLKOUTCFG_CLKOUTSEL(eClockOutSrc));
    SCG_HWA_SetClkOutCfg(u32Temp);

    /*   enable configured NVMCLK source          */
    if (eNvmClockSrc == SCG_NVMCLK_SRC_FIRC)
    {
        u32Temp |= SCG_CLKOUTCFG_NVMCLK_FIRC_MASK;
    }
    else
    {
        u32Temp |= SCG_CLKOUTCFG_NVMCLK_SIRC_MASK;
    }
    SCG_HWA_SetClkOutCfg(u32Temp);

    /* set clock out configuration information */
    SCG_SetClockOutStatus();

}

/**
 * @brief Clock source De-init
 *
 * @return SCG_StatusType function status
 */
SCG_StatusType SCG_Deinit(void)
{
    SCG_StatusType eStatusVal = SCG_CLOCK_VALID;
    SCG_Pll0Type tPll0Cfg = {
        .bEnable = false
    };
    SCG_FoscType tFoscCfg = {
        .bEnable = false
    };
    SCG_SoscType tSoscCfg = {
        .bEnable = false
    };
    SCG_Sirc32kType tSirc32kCfg = {
        .bEn = false
    };
    SCG_FircType tFircCfg =
    {
        .bEnable = true,
        .bLock = false,
        .bCm = false,
        .bTrEn = false,
        .bSten = false,
        .u8TrimSrc = 0U,
        .eDivL = SCG_ASYNCCLOCKDIV_BY4,
        .eDivM = SCG_ASYNCCLOCKDIV_BY2,
        .eDivH = SCG_ASYNCCLOCKDIV_BY1
    };

    if (SCG_HWA_GetSysClkSrc() != (uint8_t)SCG_CLOCK_SRC_FIRC)
    {
        eStatusVal = SCG_SwitchClkCtrlSrc(SCG_CLOCK_SRC_FIRC);
        if (SCG_CLOCK_ERROR == eStatusVal)
        {
            eStatusVal = SCG_SetFIRC(&tFircCfg);
            if (SCG_CLOCK_TIMEOUT != eStatusVal)
            {
                eStatusVal = SCG_SwitchClkCtrlSrc(SCG_CLOCK_SRC_FIRC);
            }
        }
    }

    if (SCG_CLOCK_VALID == eStatusVal)
    {
        /* Disable all clock source */
        (void)SCG_SetPLL0(&tPll0Cfg);
        (void)SCG_SetFOSC(&tFoscCfg);
        (void)SCG_SetSOSC(&tSoscCfg);
        (void)SCG_SetSIRC32K(&tSirc32kCfg);
    }

    return eStatusVal;
}

/**
 * @brief Clock error interrupt handler
 *
 */
void SCG_IRQHandler(void)
{
    if ((true == SCG_HWA_CheckAndClearSircClkErr()) && (NULL != s_SircClkErrNotify))
    {
        s_SircClkErrNotify();
    }

    if((true == SCG_HWA_CheckAndClearSoscClkErr()) && (NULL != s_SoscClkErrNotify))
    {
        s_SoscClkErrNotify();
    }

    if((true == SCG_HWA_CheckAndClearFoscClkErr()) && (NULL != s_FoscClkErrNotify))
    {
        s_FoscClkErrNotify();
    }

    if((true == SCG_HWA_CheckAndClearPll0ClkErr()) && (NULL != s_Pll0ClkErrNotify) )
    {
        s_Pll0ClkErrNotify();
    }

    if((true == SCG_HWA_CheckAndClearFircClkErr()) && (NULL != s_FircClkErrNotify))
    {
        s_FircClkErrNotify();
    }
}



