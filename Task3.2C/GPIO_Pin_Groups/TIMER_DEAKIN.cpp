#include <Arduino.h> // This just for Serial access.

#include <samd.h>
#include "TIMER_DEAKIN.h"

volatile long ticks = 0;

bool TIMER_DEAKIN::Config_Timer(uint8_t TimerNum, uint16_t Start, uint16_t End, double TimerRes)
{
  if(TimerNum < 3 || TimerNum > 5) return false;

  Tc *TCx = nullptr;
  switch(TimerNum)
  {
    case 3:
      TCx = TC3;
      
      Config_GCLK(0, 1, GCLK_GENCTRL_SRC_DFLL48M);
      Config_ClkCtrl(GCLK_CLKCTRL_ID_TCC2_TC3, 0);
      PM->APBCMASK.bit.TC3_ = 1;
      break;
    case 4:
      TCx = TC4;
      
      Config_GCLK(0, 1, GCLK_GENCTRL_SRC_DFLL48M);
      Config_ClkCtrl(GCLK_CLKCTRL_ID_TC4_TC5, 0);
      PM->APBCMASK.bit.TC4_ = 1;
      break;
    case 5:
      TCx = TC5;
      
      Config_GCLK(0, 1, GCLK_GENCTRL_SRC_DFLL48M);
      Config_ClkCtrl(GCLK_CLKCTRL_ID_TC4_TC5, 0);
      PM->APBCMASK.bit.TC5_ = 1;
      break;

  }

  if(TCx != nullptr)
  {
    TCx->COUNT16.CTRLA.bit.ENABLE = 0;
    while(TCx->COUNT16.STATUS.bit.SYNCBUSY);

    TCx->COUNT16.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT16_Val;

    uint32_t F_clock = 48000000;
    double desiredPeriod = TimerRes / 1000;// Convert Resolution from 0.1ms units to microseconds
    
    int prescalers[] = {1, 2, 4, 8, 16, 64, 256, 1024};
    int selectedPrescaler = 1024;  // Default prescaler
    uint16_t CompareValue = 0xFFFF;  // Default max compare value

    // Find the best prescaler and corresponding compare value
    for (int i = 0; i < 9; i++) {
        uint16_t tempCompareValue = (uint16_t)((desiredPeriod * F_clock) / prescalers[i]) - 1;
        if (tempCompareValue <= 0xFFFF) {
            selectedPrescaler = prescalers[i];
            CompareValue = tempCompareValue;
            break;
        }
    }

    // Set the prescaler based on the calculated value
    switch(selectedPrescaler)
    {
        case 1:
            TCx->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV1_Val;
            break;
        case 2:
            TCx->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV2_Val;
            break;
        case 4:
            TCx->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV4_Val;
            break;
        case 8:
            TCx->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV8_Val;
            break;
        case 16:
            TCx->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV16_Val;
            break;
        case 64:
            TCx->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV64_Val;
            break;
        case 256:
            TCx->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV256_Val;
            break;
        case 1024:
            TCx->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV1024_Val;
            break;
    }
    
    TCx->COUNT16.CTRLA.bit.WAVEGEN = TC_CTRLA_WAVEGEN_MFRQ_Val;
    TCx->COUNT16.COUNT.reg = Start;
    TCx->COUNT16.CC[0].reg = CompareValue;

    TCx->COUNT16.INTENSET.bit.MC0 = 1;
    
    TCx->COUNT16.CTRLA.bit.ENABLE = 1;
    while(TCx->COUNT16.STATUS.bit.SYNCBUSY);
  } 
  

  switch(TimerNum)
  {
    case 3:
      NVIC_SetPriority(TC3_IRQn, 0);
      NVIC_EnableIRQ(TC3_IRQn);
      break;
    case 4:
      NVIC_SetPriority(TC4_IRQn, 0);
      NVIC_EnableIRQ(TC4_IRQn);
      break;
    case 5:
      NVIC_SetPriority(TC5_IRQn, 0);
      NVIC_EnableIRQ(TC5_IRQn);
      break;
  }
  return true;
}

void TIMER_DEAKIN::Wait(double period)
{
  ticks = period;


  if(!Config_Timer(3, 0, 0, 0.1))
  {
    Serial.println("Unable to configure timer...");
    return;
  }
  while(ticks > 0){}
}

uint16_t TIMER_DEAKIN::GetTC3_Count()
{
  return TC3->COUNT16.COUNT.bit.COUNT;
}

uint16_t TIMER_DEAKIN::GetTC4_Count()
{
  return TC4->COUNT16.COUNT.bit.COUNT;
}

uint16_t TIMER_DEAKIN::GetTC5_Count()
{
  return TC5->COUNT16.COUNT.bit.COUNT;
}

void TIMER_DEAKIN::Config_GCLK(int GenID, int DivFactor, int ClkSrc)
{
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(GenID) | GCLK_GENDIV_DIV(DivFactor);
  while (GCLK->STATUS.bit.SYNCBUSY);

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(GenID) | GCLK_GENCTRL_SRC(ClkSrc) | GCLK_GENCTRL_GENEN;
  while (GCLK->STATUS.bit.SYNCBUSY);
}

void TIMER_DEAKIN::Config_ClkCtrl(int ClkID, int GenID)
{
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(ClkID) | GCLK_CLKCTRL_GEN(GenID) | GCLK_CLKCTRL_CLKEN;
  while (GCLK->STATUS.bit.SYNCBUSY);
}

void TC3_Handler(void)
{
  ticks--;
  TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
}

void TC4_Handler(void)
{
  ticks--;
  TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
}

void TC5_Handler(void)
{
  ticks--;
  TC5->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
}
