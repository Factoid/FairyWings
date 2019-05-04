#ifndef __PALETTE_H__
#define __PALETTE_H__

#include "trellis.h"

class Palette {
public:
  void Size( int s ) { paletteSize = s; }
  int numColors() { return paletteSize; }

  virtual uint32_t Color( int index ) = 0;

private:
  int paletteSize;
};

class RainbowPalette : public Palette {
public:
  uint32_t Color( int index ) {
    index %= numColors();
    
    return Wheel( index*(max_colour/numColors()) );      
  }
  
private:
  const int max_colour = 255;

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
      return trellis.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if(WheelPos < 170) {
      WheelPos -= 85;
      return trellis.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return trellis.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
};

#endif
