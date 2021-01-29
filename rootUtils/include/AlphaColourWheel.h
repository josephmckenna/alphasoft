
#ifndef __ALPHA_COLOUR_WHEEL__
#define __ALPHA_COLOUR_WHEEL__
#include <vector>
#include "TColor.h"

static int colour_list[] = {
         kRed+2, kMagenta+3, kBlue +1, kCyan +1, kGreen + 3, kYellow + 3,
         kOrange, kPink - 3, kViolet -2, kAzure - 3, kTeal -6, kSpring +8
      };
static int colour_list_size = 12;

static EColor GetColour(int i) 
{
    return (EColor)colour_list[i % colour_list_size];
}

class AlphaColourWheel
{
   private:
      int position;
   public:
      AlphaColourWheel() { position = 0;}
      EColor GetNewColour()
      {
         ++position;
         return (EColor)colour_list[position % colour_list_size];
      }
      EColor GetCurrentColour()
      {
         return (EColor)colour_list[position];
      }
};


#endif