#ifndef __COLOR_SEQUENCER_H__
#define __COLOR_SEQUENCER_H__

#include "trellis.h"

class ColorSequencer : public UIInterface {
public:
  ColorSequencer( BeatManager* beatManager ) : beatManager(beatManager), lastBeat(0), autoRandomize(false) {
    for( int i = 0; i < MAX_BEATS; ++i ) {
      paletteChoice[i] = 0;
    }
    Randomize();
  }

  void SetAutoRandomize( bool autoRandomize ) {
    this->autoRandomize = autoRandomize;
    Randomize();
  }

  bool GetAutoRandomize() {
    return autoRandomize;
  }
    
  void Randomize() {
    if( !autoRandomize ) return;
    
    for( int i = 0; i < MAX_BEATS; ++i ) {
      paletteChoice[i] = i%4;
    }
    for( int i = 0; i < MAX_BEATS; ++i ) {
      int s = rand()%(MAX_BEATS-i);
      int t = paletteChoice[i];
      paletteChoice[i] = paletteChoice[s];
      paletteChoice[s] = t;
    }
    if( manager->active(this) ) {
      updateDisplay();
    }
  }
  
  void Setup() {
    color[0] = trellis.Color(255,0,0);
    color[1] = trellis.Color(0,255,0);
    color[2] = trellis.Color(0,0,255);
    color[3] = trellis.Color(255,255,0);        

    updateDisplay();
  }
  
  void Shutdown() {
    
  }

  void OnKeyEvent( keypadEvent e ) {
    if (e.bit.EVENT == KEY_JUST_RELEASED) {
      int key = e.bit.KEY;
      ++paletteChoice[key];
      paletteChoice[key] %= 4;      
      trellis.setPixelColor(key, color[paletteChoice[key]]);
    }    
  }
  
  void Tick() {
    int activeBeat = beatManager->ActiveBeat();
    if( lastBeat != activeBeat ) {
      trellis.setPixelColor(lastBeat, color[paletteChoice[lastBeat]]);
      lastBeat = activeBeat;              
    }
    if( (millis() / 125) % 2 == 0 ) {
      trellis.setPixelColor(activeBeat, trellis.Color(255,255,255));      
    } else {
      trellis.setPixelColor(activeBeat, color[paletteChoice[activeBeat]]);            
    }
  }

  int ActivePalette() {
    int activeBeat = beatManager->ActiveBeat();
    return paletteChoice[activeBeat];
  }
  
private:
  static const int MAX_BEATS = 32;
  int paletteChoice[MAX_BEATS];
  uint32_t color[4];
  BeatManager* beatManager;
  int lastBeat;
  bool autoRandomize;

  void updateDisplay() {
    for( int i = 0; i < MAX_BEATS; ++i ) {
      trellis.setPixelColor(i, color[paletteChoice[i]]);
    }    
  }
};

#endif
