#include "Adafruit_NeoTrellisM4.h"
#include "ui_interface.h"
#include "palette.h"
#include "beat_manager.h"
#include "animation_player.h"
#include "main_menu.h"

// The NeoTrellisM4 object is a keypad and neopixel strip subclass
// that does things like auto-update the NeoPixels and stuff!
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();
int brightness = 40;

UIManager uiManager;
RainbowPalette palette;
BeatManager beatManager;
AnimationPlayer* player;

Adafruit_NeoPixel_ZeroDMA strip(32*3, PIN_WIRE_SCL, NEO_GRB + NEO_KHZ800);

void setup(){
  
  delay(250);

  strip.begin();
  strip.setBrightness(brightness);
  
  trellis.begin();
  trellis.autoUpdateNeoPixels(false);
  trellis.setBrightness(brightness);

  palette.Size(trellis.num_keys());
  player = new AnimationPlayer(&beatManager,&palette);
  player->AddAnimation( new PrideBeat() );
  player->AddAnimation( new RainbowSwirl() );
  player->AddAnimation( new LifeBeat() );
  player->AddAnimation( new DiscoBeat() );
  player->AddAnimation( new ChaseBeat() );
  player->AddAnimation( new SquareBeat() );
  player->AddAnimation( new GridBeat() );
//  player->AddAnimation( new SimpleBeat() );
  
  uiManager.push(player);
}

bool flip = false;
int c = 0;
void loop() {
  // put your main code here, to run repeatedly:
  trellis.tick();
  beatManager.Tick();
  
  if( flip && beatManager.ActiveBeat() == 0 ) {
    player->GetColorSequencer()->Randomize();
    ++c;
    if( c > 3 ) {
      player->GetAnimationSequencer()->Randomize();
      c = 0;
    }
    flip = false;
  } else if( !flip && beatManager.ActiveBeat() == 31 ) {
    flip = true;
  }


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

  const int tt[] =  { 28, 27, 20, 19, 3, 4, 11, 12,
                      29, 26, 21, 18, 2, 5, 10, 13,
                      30, 25, 22, 17, 1, 6,  9, 14,
                      31, 24, 23, 16, 0, 7,  8, 15 };
                      
  for( int i = 0; i < 32; ++i ) {
    uint32_t c = trellis.getPixelColor(i);
    for( int j = 0; j < 3; ++j ) {
      int pi = tt[i];
      strip.setPixelColor( (pi*3)+j, c );
    }
  }
  strip.show();
  trellis.show();
}
