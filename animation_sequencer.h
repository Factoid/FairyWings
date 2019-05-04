#ifndef __ANIMATION_SEQUENCER_H__
#define __ANIMATION_SEQUENCER_H__

#include "trellis.h"

class AnimationSequencer : public UIInterface {
public:
  AnimationSequencer( BeatManager* beatManager ) : beatManager(beatManager) {
    color[0] = trellis.Color(255,0,0);
    color[1] = trellis.Color(0,255,0);
    color[2] = trellis.Color(0,0,255);
    color[3] = trellis.Color(255,255,0);        
    color[4] = trellis.Color(0,255,255);
    color[5] = trellis.Color(255,0,255);
    color[6] = trellis.Color(255,64,0);
    color[7] = trellis.Color(0,255,64);              

    for( int i = 0; i < MAX_ANIM; ++i ) {
      animChoice[i] = 0;
    }
  }
  
  void Setup() {
    UpdateDisplay();
  }
  
  void Shutdown() {
   
  }
  
  void OnKeyEvent( keypadEvent e ) {
    if (e.bit.EVENT == KEY_JUST_RELEASED) {
      int key = e.bit.KEY;
      int row = key / 8;
      int col = key % 8;

      trellis.setPixelColor((row*8)+animChoice[row], color[animChoice[row]]);
      animChoice[row] = col;
    }    
  }
  
  void Tick() {
    int activeBeat = beatManager->CurrentBeat();
    int row = (activeBeat/32) % 4;
    for( int i = 0; i < MAX_ANIM; ++i ) {
      if( row == i && ((millis() / 125) % 2 == 1) ) {
        trellis.setPixelColor((i*8)+animChoice[i], color[animChoice[i]]);
      } else {
        trellis.setPixelColor((i*8)+animChoice[i], trellis.Color(255,255,255));
      }
    }
  }

  int ActiveAnimation() {
    int activeBeat = beatManager->CurrentBeat();
    int row = (activeBeat/32)%4;
    return animChoice[row];
  }

private:
  static const int MAX_ANIM = 4;
  uint32_t color[8];
  int animChoice[MAX_ANIM];
  int lastBeat;

  BeatManager* beatManager;
  
  void UpdateDisplay() {
    for( int i = 0; i < MAX_ANIM; ++i ) {
      for( int j = 0; j < 8; ++j ) {
        trellis.setPixelColor((i*8)+j, color[j]);
      }
    }
  }
};

#endif
