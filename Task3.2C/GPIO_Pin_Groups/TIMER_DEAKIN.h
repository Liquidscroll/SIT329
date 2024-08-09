#ifndef TIMER_DEAKIN_H
#define TIMER_DEAKIN_H

class TIMER_DEAKIN {
  public:
    static bool Config_Timer(uint8_t TimerNum, uint16_t Start, uint16_t End, double TimerRes);
    static void Wait(double period);
    static uint16_t GetTC3_Count();
    static uint16_t GetTC4_Count();
    static uint16_t GetTC5_Count();
    
  private:
    static void Config_GCLK(int GenID, int DivFactor, int ClkSrc);
    static void Config_ClkCtrl(int ClkID, int GenID);
};

#endif