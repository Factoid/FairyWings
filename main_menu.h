#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__
#include "draw_helpers.h"
#include "trellis.h"
#include "color_sequencer.h"
#include "animation_sequencer.h"
#include "beat_manager.h"
#include "palette_manager.h"

class MainMenu : public UIInterface {
public:
  MainMenu( BeatManager* bm, Palette* palette ) : beatManager(bm) {
    paletteManager = new PaletteManager(palette);
    colorSequencer = new ColorSequencer(beatManager);
    animSequencer = new AnimationSequencer(beatManager);
  }

  ColorSequencer* GetColorSequencer() { return colorSequencer; }
  AnimationSequencer* GetAnimationSequencer() { return animSequencer; }
  PaletteManager* GetPaletteManager() { return paletteManager; }
  
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

      if( InRect( col, row, 0, 0, 2, 2 ) ) {
        manager->push(paletteManager);
      } else if( InRect( col, row, 2, 0, 2, 2 ) ) {
        manager->push(colorSequencer);
      } else if( InRect( col, row, 4, 0, 2, 2 ) ) {
        manager->push(animSequencer);     
      } else if( row == 2 && col == 4 ) {
        int b = trellis.getBrightness();
        b += 10;
        if( b > 120 ) b = 120;
        trellis.setBrightness(b);
      } else if( row == 3 && col == 4 ) {
        int b = trellis.getBrightness();
        b -= 10;
        if( b < 10 ) b = 10;
        trellis.setBrightness(b);
      } else if( row == 2 && col == 5 ) {
        int b = strip.getBrightness();
        b += 10;
        if( b > 120 ) b = 120;
        strip.setBrightness(b);
      } else if( row == 3 && col == 5 ) {
        int b = strip.getBrightness();
        b -= 10;
        if( b < 10 ) b = 10;
        strip.setBrightness(b);
      } else if( row == 3 && col == 7 ) {
        beatManager->SampleBPM();
      } else if( row == 0 && col == 7 ) {
        colorSequencer->SetAutoRandomize( !colorSequencer->GetAutoRandomize() );
        UpdateDisplay();
      } else if( row == 1 && col == 7 ) {
        animSequencer->SetAutoRandomize( !animSequencer->GetAutoRandomize() );
        UpdateDisplay();
      }
    }
  }
  
  void Tick() {
    int c = beatManager->ActiveBeat() % 8;
    for( int i = 0; i < 4; ++i ) {
      for( int j = 2; j < 4; ++j ) {
        trellis.setPixelColor((j*8)+i, (c == ((j-2)*4)+i) ? trellis.Color(255,255,255) : 0 );
      }
    }
    trellis.setPixelColor( 31, beatManager->SampleStateColor() );        
  }

private:
  bool InRect( int c, int r, int x, int y, int w, int h ) {
    return c >= x && c < x + w && r >= y && r < y + h;
  }
    
  void UpdateDisplay() {
    trellis.fill(0);
    DrawButton(0,0,trellis.Color(255,0,0));
    DrawButton(2,0,trellis.Color(0,255,0));
    DrawButton(4,0,trellis.Color(0,0,255));
    DrawButton(4,2,trellis.Color(0,255,0),1,1);
    DrawButton(4,3,trellis.Color(255,0,0),1,1);
    DrawButton(5,2,trellis.Color(0,255,64),1,1);
    DrawButton(5,3,trellis.Color(255,64,0),1,1);

    DrawButton(7,0,trellis.Color(0,colorSequencer->GetAutoRandomize() ? 255 : 64, 0),1,1);
    DrawButton(7,1,trellis.Color(0, 0, animSequencer->GetAutoRandomize() ? 255 : 64),1,1);
  }

  BeatManager* beatManager;
  PaletteManager* paletteManager;
  ColorSequencer* colorSequencer;
  AnimationSequencer* animSequencer;
};

#endif
