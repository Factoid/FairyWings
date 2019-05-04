#ifndef __PALETTE_CHOOSER_H__
#define __PALETTE_CHOOSER_H__

#include "trellis.h"

class PaletteChooser : public UIInterface {
public:
  PaletteChooser( Palette* palette ) : palette(palette) {}

  int ActiveColor() { return blink_key; }
  void ActiveColor( int k ) { blink_key = k; }
  
  void Setup() {
    for (int i=0; i<trellis.num_keys(); i++) {
      trellis.setPixelColor(i, palette->Color(i) );
    }    
  }

  void Shutdown() {
  }
  
  void OnKeyEvent( keypadEvent e ) {
    if (e.bit.EVENT == KEY_JUST_RELEASED) {
      int key = e.bit.KEY;  // shorthand for what was pressed
      trellis.setPixelColor(blink_key, palette->Color(blink_key));
      blink_key = key;
      manager->pop();
    }    
  }

  void Tick() {
    if ( (millis() / 500) % 2 == 0 ) {
      trellis.setPixelColor(blink_key, trellis.Color(255,255,255));
    } else {
      trellis.setPixelColor(blink_key, palette->Color(blink_key));      
    }
  }
  
private:
  Palette* palette;
  int blink_key = 0;
};

#endif
