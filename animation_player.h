#ifndef __ANIMATION_PLAYER_H__
#define __ANIMATION_PLAYER_H__
#include "ui_interface.h"
#include "animations.h"
#include "trellis.h"
#include "color_sequencer.h"
#include "animation_sequencer.h"
#include "palette_manager.h"
#include "main_menu.h"

class AnimationPlayer : public UIInterface {
public:
  AnimationPlayer( BeatManager* beatManager, Palette* palette ) : beatManager(beatManager), palette(palette), numAnimations(0) {
    mainMenu = new MainMenu(beatManager,palette);
    for( int i = 0; i < 8; ++i ) {
      animations[i] = 0;
    }
  }

  void AddAnimation( Animation* anim ) {
    if( numAnimations >= 8 ) return;
    animations[numAnimations++] = anim;
  }
  
  void Setup() {  
    trellis.fill(0);
  }

  void Shutdown() {
    
  }

  void OnKeyEvent( keypadEvent e ) {
  }

  void Tick() {
    if( trellis.isPressed(0) && trellis.isPressed(7) ) {
      int c = 255;
      do {
        trellis.tick();
        delay(10);
      } while( (trellis.isPressed(0) || trellis.isPressed(7)) && (c-- > 0) );
      trellis.Adafruit_Keypad::clear();
      manager->push(mainMenu);
      return;
    }

    int activeAnim = GetAnimationSequencer()->ActiveAnimation();
    if( activeAnim >= numAnimations ) {
      trellis.fill( trellis.Color( rand()%255, rand()%255, rand()%255 ) );
    } else {
      if( animations[activeAnim] != 0 ) {
        animations[activeAnim]->Draw( beatManager->CurrentBeat(), GetPaletteManager()->GetColors(GetColorSequencer()->ActivePalette()), palette );
      } else {
        trellis.setPixelColor( activeAnim, trellis.Color( 255, 0, 0 ) );      
      }
    }
  }
  
private:
  ColorSequencer* GetColorSequencer() { 
    return mainMenu->GetColorSequencer();
  }
  AnimationSequencer* GetAnimationSequencer() {
    return mainMenu->GetAnimationSequencer();
  }
  PaletteManager* GetPaletteManager() {
    return mainMenu->GetPaletteManager();
  }
  
  MainMenu* mainMenu;
  Palette* palette;
  BeatManager* beatManager;
  Animation* animations[8];
  int numAnimations;
};

#endif
