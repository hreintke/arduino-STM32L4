/*
 * Copyright (c) 2016 Thomas Roell.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimers.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimers in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Thomas Roell, nor the names of its contributors
 *     may be used to endorse or promote products derived from this Software
 *     without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 */

#include "stm32l4xx.h"

#include "armv7m.h"

#include "stm32l4_system.h"

typedef struct _stm32l4_system_device_t {
    uint32_t lseclk;
    uint32_t hseclk;
    uint32_t sysclk;
    uint32_t hclk;
    uint32_t pclk1;
    uint32_t pclk2;
    bool     clk48;
} stm32l4_system_device_t;

static stm32l4_system_device_t stm32l4_system_device;

static volatile uint32_t * const stm32l4_system_xlate_RSTR[] = {
    &RCC->AHB1RSTR,  /* SYSTEM_PERIPH_DMA1 */
    &RCC->AHB1RSTR,  /* SYSTEM_PERIPH_DMA2 */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_GPIOA */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_GPIOB */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_GPIOC */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_GPIOD */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_GPIOE */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_GPIOF */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_GPIOG */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_GPIOH */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_USB */
    &RCC->AHB2RSTR,  /* SYSTEM_PERIPH_ADC */
    &RCC->AHB3RSTR,  /* SYSTEM_PERIPH_QSPI */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_USART1 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_USART2 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_USART3 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_UART4 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_UART5 */
    &RCC->APB1RSTR2, /* SYSTEM_PERIPH_LPUART1 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_I2C1 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_I2C2 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_I2C3 */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_SPI1 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_SPI2 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_SPI3 */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_SDMMC */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_SAI1 */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_SAI2 */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_DFSDM */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_CAN */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_TIM1 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_TIM2 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_TIM3 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_TIM4 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_TIM5 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_TIM6 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_TIM7 */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_TIM8 */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_TIM15 */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_TIM16 */
    &RCC->APB2RSTR,  /* SYSTEM_PERIPH_TIM17 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_LPTIM1 */
    &RCC->APB1RSTR2, /* SYSTEM_PERIPH_LPTIM2 */
    &RCC->APB1RSTR1, /* SYSTEM_PERIPH_DAC */
};

static uint32_t const stm32l4_system_xlate_RSTMSK[] = {
    RCC_AHB1RSTR_DMA1RST,     /* SYSTEM_PERIPH_DMA1 */
    RCC_AHB1RSTR_DMA2RST,     /* SYSTEM_PERIPH_DMA2 */
    RCC_AHB2RSTR_GPIOARST,    /* SYSTEM_PERIPH_GPIOA */
    RCC_AHB2RSTR_GPIOBRST,    /* SYSTEM_PERIPH_GPIOB */
    RCC_AHB2RSTR_GPIOCRST,    /* SYSTEM_PERIPH_GPIOC */
    RCC_AHB2RSTR_GPIODRST,    /* SYSTEM_PERIPH_GPIOD */
    RCC_AHB2RSTR_GPIOERST,    /* SYSTEM_PERIPH_GPIOE */
    RCC_AHB2RSTR_GPIOFRST,    /* SYSTEM_PERIPH_GPIOF */
    RCC_AHB2RSTR_GPIOGRST,    /* SYSTEM_PERIPH_GPIOG */
    RCC_AHB2RSTR_GPIOHRST,    /* SYSTEM_PERIPH_GPIOH */
    RCC_AHB2RSTR_OTGFSRST,    /* SYSTEM_PERIPH_USB */
    RCC_AHB2RSTR_ADCRST,      /* SYSTEM_PERIPH_ADC */
    RCC_AHB3RSTR_QSPIRST,     /* SYSTEM_PERIPH_QSPI */
    RCC_APB2RSTR_USART1RST,   /* SYSTEM_PERIPH_USART1 */
    RCC_APB1RSTR1_USART2RST,  /* SYSTEM_PERIPH_USART2 */
    RCC_APB1RSTR1_USART3RST,  /* SYSTEM_PERIPH_USART3 */
    RCC_APB1RSTR1_UART4RST,   /* SYSTEM_PERIPH_UART4 */
    RCC_APB1RSTR1_UART5RST,   /* SYSTEM_PERIPH_UART5 */
    RCC_APB1RSTR2_LPUART1RST, /* SYSTEM_PERIPH_LPUART1 */
    RCC_APB1RSTR1_I2C1RST,    /* SYSTEM_PERIPH_I2C1 */
    RCC_APB1RSTR1_I2C2RST,    /* SYSTEM_PERIPH_I2C2 */
    RCC_APB1RSTR1_I2C3RST,    /* SYSTEM_PERIPH_I2C3 */
    RCC_APB2RSTR_SPI1RST,     /* SYSTEM_PERIPH_SPI1 */
    RCC_APB1RSTR1_SPI2RST,    /* SYSTEM_PERIPH_SPI2 */
    RCC_APB1RSTR1_SPI3RST,    /* SYSTEM_PERIPH_SPI3 */
    RCC_APB2RSTR_SDMMC1RST,   /* SYSTEM_PERIPH_SDMMC */
    RCC_APB2RSTR_SAI1RST,     /* SYSTEM_PERIPH_SAI1 */
    RCC_APB2RSTR_SAI2RST,     /* SYSTEM_PERIPH_SAI2 */
    RCC_APB2RSTR_DFSDMRST,    /* SYSTEM_PERIPH_DFSDM */
    RCC_APB1RSTR1_CAN1RST,    /* SYSTEM_PERIPH_CAN */
    RCC_APB2RSTR_TIM1RST,     /* SYSTEM_PERIPH_TIM1 */
    RCC_APB1RSTR1_TIM2RST,    /* SYSTEM_PERIPH_TIM2 */
    RCC_APB1RSTR1_TIM3RST,    /* SYSTEM_PERIPH_TIM3 */
    RCC_APB1RSTR1_TIM4RST,    /* SYSTEM_PERIPH_TIM4 */
    RCC_APB1RSTR1_TIM5RST,    /* SYSTEM_PERIPH_TIM5 */
    RCC_APB1RSTR1_TIM6RST,    /* SYSTEM_PERIPH_TIM6 */
    RCC_APB1RSTR1_TIM7RST,    /* SYSTEM_PERIPH_TIM7 */
    RCC_APB2RSTR_TIM8RST,     /* SYSTEM_PERIPH_TIM8 */
    RCC_APB2RSTR_TIM15RST,    /* SYSTEM_PERIPH_TIM15 */
    RCC_APB2RSTR_TIM16RST,    /* SYSTEM_PERIPH_TIM16 */
    RCC_APB2RSTR_TIM17RST,    /* SYSTEM_PERIPH_TIM17 */
    RCC_APB1RSTR1_LPTIM1RST,  /* SYSTEM_PERIPH_LPTIM1 */
    RCC_APB1RSTR2_LPTIM2RST,  /* SYSTEM_PERIPH_LPTIM2 */
    RCC_APB1RSTR1_DAC1RST,    /* SYSTEM_PERIPH_DAC */
};

static volatile uint32_t * const stm32l4_system_xlate_ENR[] = {
    &RCC->AHB1ENR,  /* SYSTEM_PERIPH_DMA1 */
    &RCC->AHB1ENR,  /* SYSTEM_PERIPH_DMA2 */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_GPIOA */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_GPIOB */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_GPIOC */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_GPIOD */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_GPIOE */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_GPIOF */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_GPIOG */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_GPIOH */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_USB */
    &RCC->AHB2ENR,  /* SYSTEM_PERIPH_ADC */
    &RCC->AHB3ENR,  /* SYSTEM_PERIPH_QSPI */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_USART1 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_USART2 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_USART3 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_UART4 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_UART5 */
    &RCC->APB1ENR2, /* SYSTEM_PERIPH_LPUART1 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_I2C1 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_I2C2 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_I2C3 */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_SPI1 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_SPI2 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_SPI3 */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_SDMMC */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_SAI1 */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_SAI2 */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_DFSDM */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_CAN */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_TIM1 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_TIM2 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_TIM3 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_TIM4 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_TIM5 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_TIM6 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_TIM7 */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_TIM8 */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_TIM15 */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_TIM16 */
    &RCC->APB2ENR,  /* SYSTEM_PERIPH_TIM17 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_LPTIM1 */
    &RCC->APB1ENR2, /* SYSTEM_PERIPH_LPTIM2 */
    &RCC->APB1ENR1, /* SYSTEM_PERIPH_DAC */
};

static uint32_t const stm32l4_system_xlate_ENMSK[] = {
    RCC_AHB1ENR_DMA1EN,     /* SYSTEM_PERIPH_DMA1 */
    RCC_AHB1ENR_DMA2EN,     /* SYSTEM_PERIPH_DMA2 */
    RCC_AHB2ENR_GPIOAEN,    /* SYSTEM_PERIPH_GPIOA */
    RCC_AHB2ENR_GPIOBEN,    /* SYSTEM_PERIPH_GPIOB */
    RCC_AHB2ENR_GPIOCEN,    /* SYSTEM_PERIPH_GPIOC */
    RCC_AHB2ENR_GPIODEN,    /* SYSTEM_PERIPH_GPIOD */
    RCC_AHB2ENR_GPIOEEN,    /* SYSTEM_PERIPH_GPIOE */
    RCC_AHB2ENR_GPIOFEN,    /* SYSTEM_PERIPH_GPIOF */
    RCC_AHB2ENR_GPIOGEN,    /* SYSTEM_PERIPH_GPIOG */
    RCC_AHB2ENR_GPIOHEN,    /* SYSTEM_PERIPH_GPIOH */
    RCC_AHB2ENR_OTGFSEN,    /* SYSTEM_PERIPH_USB */
    RCC_AHB2ENR_ADCEN,      /* SYSTEM_PERIPH_ADC */
    RCC_AHB3ENR_QSPIEN,     /* SYSTEM_PERIPH_QSPI */
    RCC_APB2ENR_USART1EN,   /* SYSTEM_PERIPH_USART1 */
    RCC_APB1ENR1_USART2EN,  /* SYSTEM_PERIPH_USART2 */
    RCC_APB1ENR1_USART3EN,  /* SYSTEM_PERIPH_USART3 */
    RCC_APB1ENR1_UART4EN,   /* SYSTEM_PERIPH_UART4 */
    RCC_APB1ENR1_UART5EN,   /* SYSTEM_PERIPH_UART5 */
    RCC_APB1ENR2_LPUART1EN, /* SYSTEM_PERIPH_LPUART1 */
    RCC_APB1ENR1_I2C1EN,    /* SYSTEM_PERIPH_I2C1 */
    RCC_APB1ENR1_I2C2EN,    /* SYSTEM_PERIPH_I2C2 */
    RCC_APB1ENR1_I2C3EN,    /* SYSTEM_PERIPH_I2C3 */
    RCC_APB2ENR_SPI1EN,     /* SYSTEM_PERIPH_SPI1 */
    RCC_APB1ENR1_SPI2EN,    /* SYSTEM_PERIPH_SPI2 */
    RCC_APB1ENR1_SPI3EN,    /* SYSTEM_PERIPH_SPI3 */
    RCC_APB2ENR_SDMMC1EN,   /* SYSTEM_PERIPH_SDMMC */
    RCC_APB2ENR_SAI1EN,     /* SYSTEM_PERIPH_SAI1 */
    RCC_APB2ENR_SAI2EN,     /* SYSTEM_PERIPH_SAI2 */
    RCC_APB2ENR_DFSDMEN,    /* SYSTEM_PERIPH_DFSDM */
    RCC_APB1ENR1_CAN1EN,    /* SYSTEM_PERIPH_CAN */
    RCC_APB2ENR_TIM1EN,     /* SYSTEM_PERIPH_TIM1 */
    RCC_APB1ENR1_TIM2EN,    /* SYSTEM_PERIPH_TIM2 */
    RCC_APB1ENR1_TIM3EN,    /* SYSTEM_PERIPH_TIM3 */
    RCC_APB1ENR1_TIM4EN,    /* SYSTEM_PERIPH_TIM4 */
    RCC_APB1ENR1_TIM5EN,    /* SYSTEM_PERIPH_TIM5 */
    RCC_APB1ENR1_TIM6EN,    /* SYSTEM_PERIPH_TIM6 */
    RCC_APB1ENR1_TIM7EN,    /* SYSTEM_PERIPH_TIM7 */
    RCC_APB2ENR_TIM8EN,     /* SYSTEM_PERIPH_TIM8 */
    RCC_APB2ENR_TIM15EN,    /* SYSTEM_PERIPH_TIM15 */
    RCC_APB2ENR_TIM16EN,    /* SYSTEM_PERIPH_TIM16 */
    RCC_APB2ENR_TIM17EN,    /* SYSTEM_PERIPH_TIM17 */
    RCC_APB1ENR1_LPTIM1EN,  /* SYSTEM_PERIPH_LPTIM1 */
    RCC_APB1ENR2_LPTIM2EN,  /* SYSTEM_PERIPH_LPTIM2 */
    RCC_APB1ENR1_DAC1EN,    /* SYSTEM_PERIPH_DAC */
};

void stm32l4_system_periph_reset(unsigned int periph)
{
    armv7m_atomic_or(stm32l4_system_xlate_RSTR[periph], stm32l4_system_xlate_RSTMSK[periph]);
    armv7m_atomic_and(stm32l4_system_xlate_RSTR[periph], ~stm32l4_system_xlate_RSTMSK[periph]);
}

void stm32l4_system_periph_enable(unsigned int periph)
{
    armv7m_atomic_or(stm32l4_system_xlate_ENR[periph], stm32l4_system_xlate_ENMSK[periph]);
    
}

void stm32l4_system_periph_disable(unsigned int periph)
{
    armv7m_atomic_and(stm32l4_system_xlate_ENR[periph], ~stm32l4_system_xlate_ENMSK[periph]);
}

static void stm32l4_system_msi4_sysclk(void)
{
    uint32_t apb1enr1;

    FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_4WS;

    /* Select the proper voltage range */

    apb1enr1 = RCC->APB1ENR1;

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_or(&RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
    }
    
    if (PWR->CR1 & PWR_CR1_LPR)
    {
	PWR->CR1 &= ~PWR_CR1_LPR;
	
	while (PWR->SR2 & PWR_SR2_REGLPF)
	{
	}
    }

    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS) | PWR_CR1_VOS_0;

    while (PWR->SR2 & PWR_SR2_VOSF)
    {
    }

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_and(&RCC->APB1ENR1, ~RCC_APB1ENR1_PWREN);
    }

    /* Select the HSI as system clock source */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    
    /* Wait till the main HSI is used as system clock source */
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
    {
    }

    SystemCoreClock = 16000000;


    /* Select MSI @ 4MHz as system clock source.
     */

    if (!(RCC->CR & RCC_CR_MSION))
    {
	RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_6 | RCC_CR_MSIRGSEL | RCC_CR_MSION;

	while (!(RCC->CR & RCC_CR_MSIRDY))
	{
	}
    }
    else
    {
	RCC->CR = (RCC->CR & ~(RCC_CR_MSIRANGE | RCC_CR_MSIPLLEN)) | RCC_CR_MSIRANGE_6 | RCC_CR_MSIRGSEL;

	armv7m_clock_spin(500);
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_MSI;
    
    /* Wait till the main HSI is used as system clock source */
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_MSI)
    {
    }

    SystemCoreClock = 4000000;


    /* Disable PLL/PLLSAI1/PLLSAI2/HSE */
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_PLLSAI1ON | RCC_CR_PLLSAI2ON);
    
    while (RCC->CR & (RCC_CR_PLLRDY | RCC_CR_PLLSAI1RDY | RCC_CR_PLLSAI2RDY))
    {
    }
    
    if (stm32l4_system_device.hseclk <= 48000000)
    {
	RCC->CR &= ~RCC_CR_HSEON;
	
	while (RCC->CR & RCC_CR_HSERDY)
	{
	}
    }

    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2)) | (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1);

    FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_0WS;
}

/* This code below looks a tad more complicated that just going throu reset
 * and finding a way to tell whether a jump to the bootloader was requested.
 * However that involves sacrificing some of the battery backed up registers.
 * Hence this coded implements the officially recommended way of calling
 * the bootloader by disabling most of the device, reseting the clocks
 * and such.
 */
void stm32l4_system_bootloader(void)
{
    __disable_irq();

    RTC->BKP31R = 0xdeadbeef;

    NVIC_SystemReset();
    
    while (1) { }
#if 0
    /* Disable all Peripheral clocks (reset to default)
     */

    RCC->AHB1ENR = RCC_AHB1ENR_FLASHEN;
    RCC->AHB2ENR = 0;
    RCC->AHB3ENR = 0;
    RCC->APB1ENR1 = 0;
    RCC->APB1ENR2 = 0;
    RCC->APB2ENR = RCC_APB2ENR_SYSCFGEN;
    RCC->CCIPR = 0;


    /* Switch to System Memory @ 0x00000000.
     */
    SYSCFG->MEMRMP = (SYSCFG->MEMRMP & ~SYSCFG_MEMRMP_MEM_MODE) | SYSCFG_MEMRMP_MEM_MODE_0;
    RCC->APB2ENR &= ~RCC_APB2ENR_SYSCFGEN;

    /* Switch to MSI and disable LSE/HSE/PLL.
     */

    stm32l4_system_msi4_sysclk();
    
    /* Disable and clear pending interrupts (80 vectors).
     */
    SysTick->CTRL = 0;

    NVIC->ICER[0] = 0xffffffff;
    NVIC->ICER[1] = 0xffffffff;
    NVIC->ICER[2] = 0xffffffff;

    NVIC->ICPR[0] = 0xffffffff;
    NVIC->ICPR[1] = 0xffffffff;
    NVIC->ICPR[2] = 0xffffffff;

    SCB->ICSR = (SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk);
    SCB->VTOR = 0;

    /* This needs to be assembly code as GCC catches NULL 
     * dereferences ...
     */
    __asm__ volatile (
	"   mrs     r2, CONTROL                    \n"
	"   bic     r2, r2, #0x00000002            \n"
	"   msr     CONTROL, r2                    \n"
	"   mov     r2, #0x00000000                \n"
	"   ldr     r0, [r2, #0]                   \n"
	"   ldr     r1, [r2, #4]                   \n"
	"   msr     MSP, r0                        \n"
	"   isb                                    \n"
	"   bx      r1                             \n");
#endif
}

bool stm32l4_system_configure(uint32_t sysclk, uint32_t hclk, uint32_t pclk1, uint32_t pclk2, bool clk48)
{
    uint32_t fclk, fvco, fpll, fpllout, mout, nout, rout, n, r;
    uint32_t count, msirange, hpre, ppre1, ppre2, latency;
    uint32_t apb1enr1;

    if (!clk48 && ((sysclk <= 24000000) && stm32l4_system_device.clk48))
    {
	return false;
    }

    /* Detect LSE/HSE on the first pass.
     */

    if (stm32l4_system_device.lseclk == 0)
    {
	/* This is executed only on the very forst goaround, while interrupts are disabled.
	 * So RCC does not need atomics.
	 */

	RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;

	PWR->CR1 |= PWR_CR1_DBP;
	    
	while (!(PWR->CR1 & PWR_CR1_DBP))
	{
	}
	    
	if (RTC->BKP31R == 0xdeadbeef)
	{
	    RTC->BKP31R = 0x00000000;

	    /* Switch to System Memory @ 0x00000000.
	     */

	    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	    SYSCFG->MEMRMP = (SYSCFG->MEMRMP & ~SYSCFG_MEMRMP_MEM_MODE) | SYSCFG_MEMRMP_MEM_MODE_0;
	    RCC->APB2ENR &= ~RCC_APB2ENR_SYSCFGEN;

	    RCC->APB1ENR1 &= ~RCC_APB1ENR1_PWREN;

	    SCB->VTOR = 0;

	    /* This needs to be assembly code as GCC catches NULL 
	     * dereferences ...
	     */
	    __asm__ volatile (
		"   mov     r2, #0x00000000                \n"
		"   ldr     r0, [r2, #0]                   \n"
		"   ldr     r1, [r2, #4]                   \n"
		"   msr     MSP, r0                        \n"
		"   isb                                    \n"
		"   bx      r1                             \n");
	}

	if (!(RCC->BDCR & RCC_BDCR_LSEON))
	{
	    RCC->BDCR |= RCC_BDCR_LSEON;
	    
	    /* The loop below take about 8 cycles per iteration. The startup time for
	     * LSE is 5000ms. At 16MHz this corresponds to about 10000000 iterations.
	     */
	    count = 0;
	    
	    while (!(RCC->BDCR & RCC_BDCR_LSERDY))
	    {
		if (++count >= 10000000)
		{
		    RCC->BDCR &= ~RCC_BDCR_LSEON;
		    break;
		}
	    }

	    if (RCC->BDCR & RCC_BDCR_LSEON)
	    {
		/* Use LSE as source for RTC */
		RCC->BDCR |= RCC_BDCR_RTCSEL_0;
	    }
	}
	    
	if (RCC->BDCR & RCC_BDCR_LSEON)
	{
	    stm32l4_system_device.lseclk = 32768;
	}
	else
	{
	    stm32l4_system_device.lseclk = ~0ul;
	}

	/* Enable VBAT charging.
	 */
	PWR->CR4 |= PWR_CR4_VBE;

	RCC->APB1ENR1 &= ~RCC_APB1ENR1_PWREN;
    }

    if (stm32l4_system_device.hseclk == 0)
    {
	RCC->CR |= RCC_CR_HSEON;
	    
	/* The loop below take about 8 cycles per iteration. The startup time for
	 * HSE is 100ms. At 16MHz this corresponds to about 200000 iterations.
	 */
	count = 0;
	    
	while (!(RCC->CR & RCC_CR_HSERDY))
	{
	    if (++count >= 200000)
	    {
		RCC->CR &= ~RCC_CR_HSEON;
		break;
	    }
	}

	if (RCC->CR & RCC_CR_HSEON)
	{
	    /* Here we could be smart and autodetect hseclk ...
	     * but for now hardcode the 16MHz DRAGONFLY uses.
	     */
	    stm32l4_system_device.hseclk = 16000000;
	}
	else
	{
	    stm32l4_system_device.hseclk = ~0ul;
	}
    }

    /* Dummies to make the compiler happy */
    msirange = RCC_CR_MSIRANGE_6;
    mout = 2;
    nout = 8;
    rout = 2;

    if (!clk48 && (sysclk <= 24000000))
    {
	/* Range 2, use MSI */

	if      (sysclk >= 24000000) { sysclk = 24000000; msirange = RCC_CR_MSIRANGE_9; }
	else if (sysclk >= 16000000) { sysclk = 16000000; msirange = RCC_CR_MSIRANGE_8; }
	else if (sysclk >=  8000000) { sysclk =  8000000; msirange = RCC_CR_MSIRANGE_7; }
	else if (sysclk >=  4000000) { sysclk =  4000000; msirange = RCC_CR_MSIRANGE_6; }
	else if (sysclk >=  2000000) { sysclk =  2000000; msirange = RCC_CR_MSIRANGE_5; }
	else if (sysclk >=  1000000) { sysclk =  1000000; msirange = RCC_CR_MSIRANGE_4; }
	else if (sysclk >=   800000) { sysclk =   800000; msirange = RCC_CR_MSIRANGE_3; }
	else if (sysclk >=   400000) { sysclk =   400000; msirange = RCC_CR_MSIRANGE_2; }
	else if (sysclk >=   200000) { sysclk =   200000; msirange = RCC_CR_MSIRANGE_1; }
	else                         { sysclk =   100000; msirange = RCC_CR_MSIRANGE_0; }
    }
    else
    {
	/* Range 1, use HSE/PLL or MSI/PLL */

	if (sysclk < 16000000)
	{
	    sysclk = 16000000;
	}

	if (sysclk > 80000000)
	{
	    sysclk = 80000000;
	}

	/* Use fclk = 8000000, so that PLLSAI1/PLLSAI2 can
	 * assume a common clock, not matter whether MSI or HSE
	 * is driving the PLL.
	 */

	if (stm32l4_system_device.hseclk <= 48000000)
	{
	    mout = stm32l4_system_device.hseclk / 8000000; 
	}
	else
	{
	    /* MSI with 48MHz */
	    mout = 6;
	}
	
	fclk    = 8000000;
	fpllout = 0;
	nout    = 0;
	rout    = 0;
	
	for (r = 2; r <= 8; r += 2)
	{
	    n = (sysclk * r) / fclk;
	    
	    fvco = fclk * n;
	    
	    if ((n >= 8) && (n <= 86) && (fvco <= 344000000) && (fvco >= 96000000))
	    {
		fpll = fvco / r;
		
		/* Prefer higher N,R pairs for added PLL stability. */
		if (fpllout <= fpll)
		{
		    fpllout = fpll;
		    
		    nout = n;
		    rout = r;
		}
	    }
	}
	
	sysclk = fpllout;
    }

    if      (hclk >= sysclk)         { hclk = sysclk;         hpre = RCC_CFGR_HPRE_DIV1;   }
    else if (hclk >= (sysclk / 2))   { hclk = (sysclk / 2);   hpre = RCC_CFGR_HPRE_DIV2;   }
    else if (hclk >= (sysclk / 4))   { hclk = (sysclk / 4);   hpre = RCC_CFGR_HPRE_DIV4;   }
    else if (hclk >= (sysclk / 8))   { hclk = (sysclk / 8);   hpre = RCC_CFGR_HPRE_DIV8;   }
    else if (hclk >= (sysclk / 16))  { hclk = (sysclk / 16);  hpre = RCC_CFGR_HPRE_DIV16;  }
    else if (hclk >= (sysclk / 64))  { hclk = (sysclk / 64);  hpre = RCC_CFGR_HPRE_DIV64;  }
    else if (hclk >= (sysclk / 128)) { hclk = (sysclk / 128); hpre = RCC_CFGR_HPRE_DIV128; }
    else if (hclk >= (sysclk / 256)) { hclk = (sysclk / 256); hpre = RCC_CFGR_HPRE_DIV256; }
    else                             { hclk = (sysclk / 512); hpre = RCC_CFGR_HPRE_DIV512; }
    
    if      (pclk1 >= hclk)       { pclk1 = hclk;        ppre1 = RCC_CFGR_PPRE1_DIV1;  }
    else if (pclk1 >= (hclk / 2)) { pclk1 = (hclk / 2);  ppre1 = RCC_CFGR_PPRE1_DIV2;  }
    else if (pclk1 >= (hclk / 4)) { pclk1 = (hclk / 4);  ppre1 = RCC_CFGR_PPRE1_DIV4;  }
    else if (pclk1 >= (hclk / 8)) { pclk1 = (hclk / 8);  ppre1 = RCC_CFGR_PPRE1_DIV8;  }
    else                          { pclk1 = (hclk / 16); ppre1 = RCC_CFGR_PPRE1_DIV16; }

    if      (pclk2 >= hclk)       { pclk2 = hclk;        ppre2 = RCC_CFGR_PPRE2_DIV1;  }
    else if (pclk2 >= (hclk / 2)) { pclk2 = (hclk / 2);  ppre2 = RCC_CFGR_PPRE2_DIV2;  }
    else if (pclk2 >= (hclk / 4)) { pclk2 = (hclk / 4);  ppre2 = RCC_CFGR_PPRE2_DIV4;  }
    else if (pclk2 >= (hclk / 8)) { pclk2 = (hclk / 8);  ppre2 = RCC_CFGR_PPRE2_DIV8;  }
    else                          { pclk2 = (hclk / 16); ppre2 = RCC_CFGR_PPRE2_DIV16; }

    /* #### Add code to prepare the clock switch, and if it fails cancel it.
     */

    /* First switch to HSI as system clock.
     */

    apb1enr1 = RCC->APB1ENR1;

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_or(&RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
    }

    /* Select Range 1 to switch clocks */

    if (PWR->CR1 & PWR_CR1_LPR)
    {
	PWR->CR1 &= ~PWR_CR1_LPR;
	
	while (PWR->SR2 & PWR_SR2_REGLPF)
	{
	}
    }

    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS) | PWR_CR1_VOS_0;

    while (PWR->SR2 & PWR_SR2_VOSF)
    {
    }

    FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_4WS;
      
    /* Make sure HSI is on, switch to HSI as clock source */
    RCC->CR |= RCC_CR_HSION;
    
    while (!(RCC->CR & RCC_CR_HSIRDY))
    {
    }

    /* Select the HSI as system clock source */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    
    /* Wait till the main HSI is used as system clock source */
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
    {
    }

    SystemCoreClock = 16000000;


    /* Disable PLL/PLLSAI1/PLLSAI2 */
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_PLLSAI1ON | RCC_CR_PLLSAI2ON);

    while (RCC->CR & (RCC_CR_PLLRDY | RCC_CR_PLLSAI1RDY | RCC_CR_PLLSAI2RDY))
    {
    }

    /* Set HCLK/PCLK1/PCLK2 prescalers */
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2)) | (hpre | ppre1 | ppre2);


    if (!clk48 && (sysclk <= 24000000))
    {
	/* Range 2, use MSI */

	/* Disable HSE */
	if (stm32l4_system_device.hseclk <= 48000000)
	{
	    RCC->CR &= ~RCC_CR_HSEON;

	    while (RCC->CR & RCC_CR_HSERDY)
	    {
	    }
	}

	if (!(RCC->CR & RCC_CR_MSION))
	{
	    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | msirange | RCC_CR_MSIRGSEL | RCC_CR_MSION;
	    
	    while (!(RCC->CR & RCC_CR_MSIRDY))
	    {
	    }

	    if (stm32l4_system_device.lseclk == 32768)
	    {
		/* Enable the MSI PLL */
		RCC->CR |= RCC_CR_MSIPLLEN;
	    }
	}
	else
	{
	    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | msirange | RCC_CR_MSIRGSEL;

	    armv7m_clock_spin(500);
	}
	
	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_MSI;
	
	/* Wait till the main MSI is used as system clock source */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_MSI)
	{
	}

	SystemCoreClock = sysclk;


	PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS) | PWR_CR1_VOS_1;

	while (PWR->SR2 & PWR_SR2_VOSF)
	{
	}

	/* Entry LPRUN is sysclk is low enough */
	if (sysclk <= 2000000)
	{
	    PWR->CR1 |= PWR_CR1_LPR;
	
	    while (!(PWR->SR2 & PWR_SR2_REGLPF))
	    {
	    }
	}
    }
    else
    {
	/* Range 1, use HSE/PLL or MSI/PLL */

	/* Be careful not to change HSE/MSI settings which might be shared
	 * with PLLSAI1/PLLSAI2.
	 */
	if (stm32l4_system_device.hseclk <= 48000000)
	{
	    if (!(RCC->CR & RCC_CR_HSEON))
	    {
		RCC->CR |= RCC_CR_HSEON;
	    
		while (!(RCC->CR & RCC_CR_HSERDY))
		{
		}
	    }
	}

	if (!(RCC->CR & RCC_CR_HSEON) || stm32l4_system_device.clk48)
	{
	    if (!((RCC->CR & (RCC_CR_MSION | RCC_CR_MSIRANGE)) == (RCC_CR_MSION | RCC_CR_MSIRANGE_11)))
	    {
		if (!(RCC->CR & RCC_CR_MSION))
		{
		    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_11 | RCC_CR_MSIRGSEL | RCC_CR_MSION;
		    
		    while (!(RCC->CR & RCC_CR_MSIRDY))
		    {
		    }
		    
		    if (stm32l4_system_device.lseclk == 32768)
		    {
			/* Enable the MSI PLL */
			RCC->CR |= RCC_CR_MSIPLLEN;
		    }
		}
		else
		{
		    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_11 | RCC_CR_MSIRGSEL;
		    
		    armv7m_clock_spin(500);
		}
	    }
	}
	else
	{
	    /* ERRATA 2.1.15. WAR: Switch MSI to <= 16MHz before turing off */
	    RCC->CR = (RCC->CR & ~(RCC_CR_MSIRANGE | RCC_CR_MSIPLLEN)) | RCC_CR_MSIRANGE_6 | RCC_CR_MSIRGSEL;

	    armv7m_clock_spin(500);

	    RCC->CR &= ~RCC_CR_MSION;
		    
	    while (RCC->CR & RCC_CR_MSIRDY)
	    {
	    }
	}
	
	/* Configure the main PLL */
	RCC->PLLCFGR = (((mout -1) << 4) |
			(nout << 8) |
			(((rout >> 1) -1) << 25) |
			((RCC->CR & RCC_CR_HSEON) ? RCC_PLLCFGR_PLLSRC_HSE : RCC_PLLCFGR_PLLSRC_MSI) |
			RCC_PLLCFGR_PLLREN);
	
	/* Enable the main PLL */
	RCC->CR |= RCC_CR_PLLON;
	
	/* Wait till the main PLL is ready */
	while((RCC->CR & RCC_CR_PLLRDY) == 0)
	{
	}
	
	/* Select the main PLL as system clock source */
	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
	
	/* Wait till the main PLL is used as system clock source */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
	{
	}

	SystemCoreClock = sysclk;
    }

    if (!clk48 && (sysclk <= 24000000))
    {
	if      (hclk <=  6000000) { latency = FLASH_ACR_LATENCY_0WS; }
	else if (hclk <= 12000000) { latency = FLASH_ACR_LATENCY_1WS; }
	else if (hclk <= 18000000) { latency = FLASH_ACR_LATENCY_2WS; }
	else                       { latency = FLASH_ACR_LATENCY_3WS; }

	FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | latency;
    }
    else
    {
	if      (hclk <= 16000000) { latency = FLASH_ACR_LATENCY_0WS; }
	else if (hclk <= 32000000) { latency = FLASH_ACR_LATENCY_1WS; }
	else if (hclk <= 48000000) { latency = FLASH_ACR_LATENCY_2WS; }
	else if (hclk <= 64000000) { latency = FLASH_ACR_LATENCY_3WS; }
	else                       { latency = FLASH_ACR_LATENCY_4WS; }

	FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | latency;
    }

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_and(&RCC->APB1ENR1, ~RCC_APB1ENR1_PWREN);
    }

    stm32l4_system_device.sysclk = sysclk;
    stm32l4_system_device.hclk = hclk;
    stm32l4_system_device.pclk1 = pclk1;
    stm32l4_system_device.pclk2 = pclk2;

    /* #### Add code to complete the switch */

    return true;
}

bool stm32l4_system_clk48_enable(void)
{
    if (((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) || (stm32l4_system_device.lseclk != 32768))
    {
	return false;
    }

    if (RCC->CR & RCC_CR_HSEON)
    {
	if (!(RCC->CR & RCC_CR_MSION))
	{
	    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_11 | RCC_CR_MSIRGSEL | RCC_CR_MSION;
	    
	    while (!(RCC->CR & RCC_CR_MSIRDY))
	    {
	    }
	    
	    if (stm32l4_system_device.lseclk == 32768)
	    {
		/* Enable the MSI PLL */
		RCC->CR |= RCC_CR_MSIPLLEN;
	    }
	}
	else
	{
	    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_11 | RCC_CR_MSIRGSEL;
	    
	    armv7m_clock_spin(500);
	}
    }

    /* Switch CLK48 to MSI */
    RCC->CCIPR |= RCC_CCIPR_CLK48SEL;

    stm32l4_system_device.clk48 = true;

    return true;
}

bool stm32l4_system_clk48_disable(void)
{
    if (((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) || (stm32l4_system_device.lseclk != 32768))
    {
	return false;
    }

    if (RCC->CR & RCC_CR_HSEON)
    {
	/* ERRATA 2.1.15. WAR: Switch MSI to <= 16MHz before turing off */
	RCC->CR = (RCC->CR & ~(RCC_CR_MSIRANGE | RCC_CR_MSIPLLEN)) | RCC_CR_MSIRANGE_6 | RCC_CR_MSIRGSEL;

	armv7m_clock_spin(500);

	RCC->CR &= ~RCC_CR_MSION;
		    
	while (RCC->CR & RCC_CR_MSIRDY)
	{
	}
    }

    /* Switch CLK48 to off */
    RCC->CCIPR &= ~RCC_CCIPR_CLK48SEL;

    stm32l4_system_device.clk48 = false;

    return true;
}

uint32_t stm32l4_system_sysclk(void)
{
    return stm32l4_system_device.sysclk;
}

uint32_t stm32l4_system_hclk(void)
{
    return stm32l4_system_device.hclk;
}

uint32_t stm32l4_system_pclk1(void)
{
    return stm32l4_system_device.pclk1;
}

uint32_t stm32l4_system_pclk2(void)
{
    return stm32l4_system_device.pclk2;
}

bool stm32l4_system_suspend(void)
{
    uint32_t apb1enr1;

    /* #### Add code here that calls STOP_ENTER, and if that fails calls out STOP_CANCEL */
       
    /* Disable FLASH in sleep/deepsleep */
    FLASH->ACR |= FLASH_ACR_SLEEP_PD;

    if (stm32l4_system_device.sysclk <= 24000000)
    {
	/* Select HSI as wakeup clock */

	RCC->CR |= RCC_CR_HSIASFS;
	RCC->CFGR &= ~RCC_CFGR_STOPWUCK;
    }
    else
    {
	/* There are a few ERRATA which make it reasonable to simply 
	 * switch to HSI and turn off PLL/PLLSAI1/PLLSAI2/MSE/MSI manually.
	 */

	/* Select the HSI as system clock source */
	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
	
	/* Wait till the main HSI is used as system clock source */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
	{
	}
	
	/* Select MSI @ 4MHz as system clock source.
	 */
	
	if (RCC->CR & RCC_CR_MSION)
	{
	    /* ERRATA 2.1.15. WAR: Switch MSI to <= 16MHz before turing off */
	    RCC->CR = (RCC->CR & ~(RCC_CR_MSIRANGE | RCC_CR_MSIPLLEN)) | RCC_CR_MSIRANGE_6 | RCC_CR_MSIRGSEL;

	    armv7m_clock_spin(500);
	    
	    RCC->CR &= ~RCC_CR_MSION;
	    
	    while (RCC->CR & RCC_CR_MSIRDY)
	    {
	    }
	}

	/* Disable PLL/PLLSAI1/PLLSAI2/HSE */
	RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_PLLSAI1ON | RCC_CR_PLLSAI2ON);
	
	while (RCC->CR & (RCC_CR_PLLRDY | RCC_CR_PLLSAI1RDY | RCC_CR_PLLSAI2RDY))
	{
	}
	
	if (stm32l4_system_device.hseclk <= 48000000)
	{
	    RCC->CR &= ~RCC_CR_HSEON;
	    
	    while (RCC->CR & RCC_CR_HSERDY)
	    {
	    }
	}

	/* Select HSI as wakeup clock */
	RCC->CFGR |= RCC_CFGR_STOPWUCK;
    }

    /* Set up STOP1 */

    apb1enr1 = RCC->APB1ENR1;

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_or(&RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
    }
    
    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_LPMS) | PWR_CR1_LPMS_STOP1;

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_and(&RCC->APB1ENR1, ~RCC_APB1ENR1_PWREN);
    }

    return true;
}

bool stm32l4_system_restore(void)
{
    if (stm32l4_system_device.sysclk <= 24000000)
    {
    }
    else
    {
	if (stm32l4_system_device.hseclk <= 48000000)
	{
	    RCC->CR |= RCC_CR_HSEON;
	    
	    while (!(RCC->CR & RCC_CR_HSERDY))
	    {
	    }
	}

	if (!(RCC->CR & RCC_CR_HSEON) || stm32l4_system_device.clk48)
	{
	    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_11 | RCC_CR_MSIRGSEL | RCC_CR_MSION;
		    
	    while (!(RCC->CR & RCC_CR_MSIRDY))
	    {
	    }
		    
	    if (stm32l4_system_device.lseclk == 32768)
	    {
		/* Enable the MSI PLL */
		RCC->CR |= RCC_CR_MSIPLLEN;
	    }
	}

	/* Enable the main PLL */
	RCC->CR |= RCC_CR_PLLON;
	
	/* Wait till the main PLL is ready */
	while((RCC->CR & RCC_CR_PLLRDY) == 0)
	{
	}
	
	/* Select the main PLL as system clock source */
	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
	
	/* Wait till the main PLL is used as system clock source */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
	{
	}
    }

    /* Enable FLASH in sleep/deepsleep */
    FLASH->ACR &= ~FLASH_ACR_SLEEP_PD;

    /* #### Add code here that calls STOP_LEAVE */

    return true;
}

bool stm32l4_system_stop(void)
{
    if (!stm32l4_system_suspend())
    {
	return false;
    }

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __SEV();
    __WFE();
    __WFE();

    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

    return stm32l4_system_restore();
}

bool stm32l4_system_standby(void)
{
    uint32_t apb1enr1;

    /* #### Add code here that calls STANDBY_ENTER, and if that fails calls out STANDBY_CANCEL */

    /* ERRATA 2.1.15. WAR: Switch MSI to 4MHz before entering low power mode */

    stm32l4_system_msi4_sysclk();

    apb1enr1 = RCC->APB1ENR1;

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_or(&RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
    }
    
    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_LPMS) | PWR_CR1_LPMS_STANDBY;

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_and(&RCC->APB1ENR1, ~RCC_APB1ENR1_PWREN);
    }

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __WFI();

    return true;
}

bool stm32l4_system_shutdown(void)
{
    uint32_t apb1enr1;

    /* #### Add code here that calls SHUTDOWN_ENTER, and if that fails calls out SHUTDOWN_CANCEL */

    /* ERRATA 2.1.15. WAR: Switch MSI to 4MHz before entering low power mode */

    stm32l4_system_msi4_sysclk();

    apb1enr1 = RCC->APB1ENR1;

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_or(&RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
    }
    
    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_LPMS) | PWR_CR1_LPMS_SHUTDOWN;

    if (!(apb1enr1 & RCC_APB1ENR1_PWREN))
    {
	armv7m_atomic_and(&RCC->APB1ENR1, ~RCC_APB1ENR1_PWREN);
    }

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __WFI();

    return true;
}
