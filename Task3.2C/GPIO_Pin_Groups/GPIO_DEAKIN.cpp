#include <Arduino.h>

#include "GPIO_DEAKIN.h"
#include "TIMER_DEAKIN.h"

bool GPIO_DEAKIN::Check_Pin_Input(char PortNum, char PinNum)
{
  if (PortNum < 'A' || PortNum > 'B')
  {
    Serial.println("Port Error: Invalid Port");
    return false;
  }
  if(PinNum > 31) 
  {
    Serial.println("Pin Error: Invalid Pin Number");
    return false;
  }

  return true;
}

PortGroup* GPIO_DEAKIN::Get_PortGroup(char PortNum)
{
  if (PortNum == 'A')
  {
    return &(PORT->Group[0]);
  }
  else if (PortNum == 'B')
  {
    return &(PORT->Group[1]);
  }

  return nullptr;
}


bool GPIO_DEAKIN::Config_GPIO(char PortNum, char PinNum, char Mode)
{
  if(!Check_Pin_Input(PortNum, PinNum)) return false;
  

  PortGroup *pg = Get_PortGroup(PortNum);

  if(pg == nullptr) 
  {
    Serial.println("Port Error: Port Group not found");
    return false;
  }

  if(Mode == INPUT)
  {
    pg->DIRCLR.reg = (1 << PinNum); // Configure as input by clearing direction
  }
  else if(Mode == OUTPUT)
  {
    pg->DIRSET.reg = (1 << PinNum); // Configure as output by setting direction
  }
  else 
  {
    Serial.println("Config Error: Invalid Mode.");
    return false;
  }

  return true;

}

bool GPIO_DEAKIN::Write_GPIO(char PortNum, char PinNum, bool State)
{
  if(!Check_Pin_Input(PortNum, PinNum)) return false;

  PortGroup *pg = Get_PortGroup(PortNum);

  if(pg == nullptr) 
  {
    Serial.println("Port Error: Port Group not found");
    return false;
  }

  // Check if directional bit for pin is not set, if bit 
  // is not set then this Pin is configured to be an input.
  if((pg->DIR.reg & (1 << PinNum)) == 0) 
  {
    Serial.println("Cannot write to an input pin.");
    return false;
  }

  if(State == HIGH)
  {
    pg->OUTSET.reg = (1 << PinNum); // Set output bit, making pin HIGH
  }
  else
  {
    pg->OUTCLR.reg = (1 << PinNum); // Clear output bit, making pin LOW
  }

  return true;
}
bool GPIO_DEAKIN::Read_GPIO(char PortNum, char PinNum)
{
  if(!Check_Pin_Input(PortNum, PinNum)) return false;

  PortGroup *pg = Get_PortGroup(PortNum);

  if(pg == nullptr) 
  {
    Serial.println("Port Error: Port Group not found.");
    return false;
  }
  // Return the state of the pin, indicated by the appropriate
  // bit in the IN register
  return (pg->IN.reg & (1 << PinNum)) != 0;
}

// Function to get the PortGroup and pin number based on the logical pin number
PortGroup* GPIO_DEAKIN::Get_PortGroup_For_Pin(int Pin, int *PinNum)
{
  if (Pin >= PINCOUNT_fn())
  {
    Serial.println("Invalid Pin Number");
    return nullptr;
  }

  // Get the port and pin number from the pin description array
  EPortType port = g_APinDescription[Pin].ulPort;
  *PinNum = g_APinDescription[Pin].ulPin;

  if(port == PORTA) return &(PORT->Group[0]);
  if(port == PORTB) return &(PORT->Group[1]);
  else return nullptr;
}

bool GPIO_DEAKIN::Config_GPIO_Pins(char *PinArray, char Mode)
{
  uint32_t maskA = 0, maskB = 0;

   // Loop through each pin in the array and build the masks for PORTA and PORTB
  for(int i = 0; i < 8; i++)
  {
    EPortType port = g_APinDescription[PinArray[i]].ulPort;
    uint32_t pin = g_APinDescription[PinArray[i]].ulPin;
    if(port == PORTA) maskA |= (1 << pin);
    else if(port == PORTB) maskB |= (1 << pin);
    else
    {
      Serial.println("Port Error: Incorrect Port");
      return false;
    }
  }

  PortGroup *pgA = Get_PortGroup('A');
  PortGroup *pgB = Get_PortGroup('B');
  // Set the direction of pins based on the mode
  if(Mode == INPUT)
  {
    if(pgA) pgA->DIRCLR.reg = maskA; // Configure PORTA pins as input
    if(pgB) pgB->DIRCLR.reg = maskB; // Configure PORTB pins as input
  }
  else if (Mode == OUTPUT)
  {
    if(pgA) pgA->DIRSET.reg = maskA; // Configure PORTA pins as output
    if(pgB) pgB->DIRSET.reg = maskB; // Configure PORTB pins as output
  }
  else
  {
    Serial.println("Config Error: Invalid Mode");
    return false;
  }

  return true;

}

bool GPIO_DEAKIN::GPIO_Display_Pattern(char *PinArray, char GPIOPattern)
{
  uint32_t maskA_SET = 0, maskA_CLR = 0, maskB_SET = 0, maskB_CLR = 0;

   // Loop through each pin and determine whether to set or clear the pin based on the pattern
  for(int i = 0; i < 8; i++)
  {
    EPortType port = g_APinDescription[PinArray[i]].ulPort;
    uint32_t pin = g_APinDescription[PinArray[i]].ulPin;
    if(GPIOPattern & (1 << (7 - i)))
    {
      if(port == PORTA)
      {
        maskA_SET |= (1 << pin); // Set the pin if the corresponding bit in the pattern is 1
      }
      else if (port == PORTB)
      {
        maskB_SET |= (1 << pin); // Set the pin if the corresponding bit in the pattern is 1
      }
    }
    else
    {
      if(port == PORTA)
      {
        maskA_CLR |= (1 << pin); // Clear the pin if the corresponding bit in the pattern is 0
      }
      else if (port == PORTB)
      {
        maskB_CLR |= (1 << pin); // Clear the pin if the corresponding bit in the pattern is 0
      }
    }
  }

  PortGroup *pgA = Get_PortGroup('A');
  PortGroup *pgB = Get_PortGroup('B');

  // Set or clear the pins based on the masks created
  if (pgA) 
  {
    pgA->OUTSET.reg = maskA_SET;
    pgA->OUTCLR.reg = maskA_CLR;
  }
  if (pgB) 
  {
    pgB->OUTSET.reg = maskB_SET;
    pgB->OUTCLR.reg = maskB_CLR;
  }

  return true;

}

void GPIO_DEAKIN::GPIO_Sequential_Lights(char *PinArray, double Interval)
{
  while(true)
  {
    int i = 0;
    // Loop to light up pins from first to last
    for(; i < 8; i++)
    {
      char PortNum = g_APinDescription[PinArray[i]].ulPort == PORTA ? 'A' : 'B';
      char PinNum = (char)g_APinDescription[PinArray[i]].ulPin;

      Write_GPIO(PortNum, PinNum, HIGH);
      TIMER_DEAKIN::Wait(Interval);
      Write_GPIO(PortNum, PinNum, LOW);
    }
    i--;
    // Loop to light up pins from last to first
    for(; i >= 0; i--)
    {
      char PortNum = g_APinDescription[PinArray[i]].ulPort == PORTA ? 'A' : 'B';
      char PinNum = (char)g_APinDescription[PinArray[i]].ulPin;
      Write_GPIO(PortNum, PinNum, HIGH);
      TIMER_DEAKIN::Wait(Interval);
      Write_GPIO(PortNum, PinNum, LOW);
    }
  }

}
