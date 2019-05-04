#ifndef __PALETTE_MANAGER_H__
#define __PALETTE_MANAGER_H__
#include "palette_chooser.h"
#include "trellis.h"

class PaletteManager : public UIInterface {
public:
  PaletteManager( Palette* p ) : palette(p), chooser(NULL) {
    for( int pal = 0; pal < 4; ++pal ){
      for( int col = 0; col < 4; ++col ) {
        paletteChoices[pal][col] = (pal*8)+(col*2);
      }
    }    
  }
  
  void Setup() {
    if( chooser ) {
      paletteChoices[selectedPal][selectedCol] = chooser->ActiveColor();
      delete chooser;
      chooser = NULL;
    }
    for( int pal = 0; pal < 4; ++pal ){
      for( int col = 0; col < 4; ++col ) {
        trellis.setPixelColor( (col*8)+(pal*2), palette->Color(paletteChoices[pal][col]) );
        trellis.setPixelColor( (col*8)+(pal*2)+1, palette->Color(paletteChoices[pal][col]) );
      }
    }
  }

  void Shutdown() {
    
  }

  void OnKeyEvent( keypadEvent e ) {
    if (e.bit.EVENT == KEY_JUST_RELEASED) {
      int key = e.bit.KEY;  // shorthand for what was pressed
      
      int column = key % 8;
      int row = key / 8;
      
      selectedCol = row;
      selectedPal = (column/2) % 4;
      
      chooser = new PaletteChooser(palette);
      chooser->ActiveColor(paletteChoices[selectedPal][selectedCol]);
      manager->push(chooser);
    }
  }
  
  void Tick() {
    
  }

  int* GetColors( int palette ) {
    return paletteChoices[palette];
  }
  
private:
  int paletteChoices[4][4];
  Palette* palette;
  PaletteChooser* chooser;
  int selectedCol;
  int selectedPal;
};

#endif
