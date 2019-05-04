#include "Adafruit_NeoTrellisM4.h"
#include "ui_interface.h"
#include "palette.h"
#include "beat_manager.h"
#include "animation_player.h"
#include "main_menu.h"

// The NeoTrellisM4 object is a keypad and neopixel strip subclass
// that does things like auto-update the NeoPixels and stuff!
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();
int brightness = 60;

UIManager uiManager;
RainbowPalette palette;
BeatManager beatManager;
AnimationPlayer* player;
MainMenu* mainMenu;

void setup(){
  delay(250);
  
  trellis.begin();
  trellis.autoUpdateNeoPixels(false);
  trellis.setBrightness(brightness);

  palette.Size(trellis.num_keys());
  //mainMenu = new MainMenu(&beatManager,&palette);
  player = new AnimationPlayer(&beatManager,&palette);
  player->AddAnimation( new ChaseBeat() );
  player->AddAnimation( new GridBeat() );
  player->AddAnimation( new SquareBeat() );
  player->AddAnimation( new DiscoBeat() );
  player->AddAnimation( new SimpleBeat() );
  
  uiManager.push(player);
}
  
void loop() {
  // put your main code here, to run repeatedly:
  trellis.tick();
  beatManager.Tick();

  if( uiManager.numUI() > 1 ) {    
    if( trellis.isPressed(0) && trellis.isPressed(7) ) {
      uiManager.pop();
      int c = 255;
      do {
        trellis.tick();
        delay(10);
      } while( (trellis.isPressed(0) || trellis.isPressed(7)) && c-- > 0);
      trellis.Adafruit_Keypad::clear();
    }
  }
  
  while (trellis.available()){
    keypadEvent e = trellis.read();
    uiManager.active_menu()->OnKeyEvent(e);
  }

  uiManager.active_menu()->Tick();
  trellis.show();
  delay(10);
}
