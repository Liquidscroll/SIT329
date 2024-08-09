#ifndef GPIO_DEAKIN_H
#define GPIO_DEAKIN_H

#include <samd.h>

class GPIO_DEAKIN {
  public:
    static bool Config_GPIO(char PortNum, char PinNum, char Mode);
    static bool Write_GPIO(char PortNum, char PinNum, bool State);
    static bool Read_GPIO(char PortNum, char PinNum);

    static bool Config_GPIO_Pins(char *PinArray, char Mode);
    static bool GPIO_Display_Pattern(char *PinArray, char GPIOPattern);
    static void GPIO_Sequential_Lights(char *PinArray, double Interval);

  private:
    static bool Check_Pin_Input(char PortNum, char PinNum);
    static PortGroup* Get_PortGroup(char PortNum);
    static PortGroup* Get_PortGroup_For_Pin(int Pin, int *PinNum);
};

#endif