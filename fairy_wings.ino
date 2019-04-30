#include "Adafruit_NeoTrellisM4.h"

// The NeoTrellisM4 object is a keypad and neopixel strip subclass
// that does things like auto-update the NeoPixels and stuff!
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();
int brightness = 60;

void DrawButton( int x, int y, uint32_t color, int w = 2, int h = 2 )
{
  for( int cx = x; cx < x + w; ++cx ) {
    for( int cy = y; cy < y + h; ++cy ) {
      trellis.setPixelColor((cy*8)+cx, color);
    }
  }       
}

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

class UIManager;

class UIInterface {
public:
  virtual void Setup() = 0;
  virtual void Shutdown() = 0;
  virtual void OnKeyEvent( keypadEvent e ) = 0;
  virtual void Tick() = 0;

  static void SetManager( UIManager* manager ) { UIInterface::manager = manager; }
protected:
  static UIManager* manager; 
};
UIManager* UIInterface::manager = 0;

class UIManager {
public:
  UIManager() { UIInterface::SetManager(this); }
  UIInterface* active_menu() { 
    if( activeInterface < 0 ) return NULL;
    return stack[activeInterface];
  }
  
  void push( UIInterface* interface ) {
    if( activeInterface >= maxStack - 1 ) return;
    if( activeInterface >= 0 ) stack[activeInterface]->Shutdown();
    ++activeInterface;
    stack[activeInterface] = interface;
    stack[activeInterface]->Setup();
  }
  
  void pop() {
    stack[activeInterface]->Shutdown();
    stack[activeInterface] = NULL;
    --activeInterface;
    if( activeInterface >= 0 ) stack[activeInterface]->Setup();
  }

  int numUI() {
    return activeInterface + 1;
  }
  
private:
  static const int maxStack = 12;
  UIInterface* stack[maxStack];
  int activeInterface = -1;
};

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

class BeatManager {
public:
  BeatManager() : bpm(110.0f), sampling(false), lastSample(0) {    
  }

  void Tick() {
    unsigned long now = millis();
    float delta = (now - lastTime)/1000.0f;
    lastTime = now;
    activeBeat += delta * (bpm/60.0f);

    if( sampling && now - samples[sampleIndex-1] > 2000.0f ) {
      sampling = false;
    }
  }

  int ActiveBeat() { return (int)activeBeat%32; }
  float CurrentBeat() { return activeBeat; }

  void SetBPM(float value) { bpm = value; }
  float GetBPM() { return bpm; }

  void SampleBPM() {
    if( !sampling || sampleIndex == 8 ) {
      sampling = true;
      sampleIndex = 0;
    }

    if( sampleIndex < 8 ) {
      samples[sampleIndex] = millis();
      ++sampleIndex;
      if( sampleIndex == 8 ) {
        UpdateBPMFromSamples();
      }
    }
  }

  uint32_t SampleStateColor() {
    if( !sampling ) return trellis.Color(5,5,5);
    return sampleIndex < 8 ? trellis.Color(255,0,0) : trellis.Color(0,255,0);    
  }
  
private:
  void UpdateBPMFromSamples() {
    float delta = 0;
    for( int i = 0; i < 7; ++i ) {
      delta += samples[i+1] - samples[i];
    }
    delta /= 7;
    SetBPM( 1000.0f/delta*60.0f );
    activeBeat = (int)(activeBeat + 1);
  }
  
  unsigned long lastTime;
  float activeBeat;
  float bpm;

  bool sampling;
  unsigned long lastSample;
  byte sampleIndex;
  unsigned long samples[8];
};

class ColorSequencer : public UIInterface {
public:
  ColorSequencer( BeatManager* beatManager ) : beatManager(beatManager), lastBeat(0) {
    for( int i = 0; i < MAX_BEATS; ++i ) {
      paletteChoice[i] = 0;
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

  void updateDisplay() {
    for( int i = 0; i < MAX_BEATS; ++i ) {
      trellis.setPixelColor(i, color[paletteChoice[i]]);
    }    
  }
};

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
    int activeBeat = beatManager->ActiveBeat();
    int row = activeBeat / 8;
    for( int i = 0; i < MAX_ANIM; ++i ) {
      if( row == i && ((millis() / 125) % 2 == 1) ) {
        trellis.setPixelColor((i*8)+animChoice[i], color[animChoice[i]]);
      } else {
        trellis.setPixelColor((i*8)+animChoice[i], trellis.Color(255,255,255));
      }
    }
  }

  int ActiveAnimation() {
    int activeBeat = beatManager->ActiveBeat();
    int row = activeBeat / 8;
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
      } else if( row == 2 && col == 5 ) {
        beatManager->SetBPM( beatManager->GetBPM() + 5 );
      } else if( row == 3 && col == 5 ) {
        beatManager->SetBPM( beatManager->GetBPM() - 5 );
      } else if( row == 2 && col == 4 ) {
        beatManager->SetBPM( beatManager->GetBPM() + 1 );
      } else if( row == 3 && col == 4 ) {
        beatManager->SetBPM( beatManager->GetBPM() - 1 );
      } else if( row == 3 && col == 7 ) {
        beatManager->SampleBPM();
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
    DrawButton(4,2,trellis.Color(0,255,0),1);
    DrawButton(4,3,trellis.Color(255,0,0),1);
    DrawButton(5,2,trellis.Color(0,255,64),1);
    DrawButton(5,3,trellis.Color(255,64,0),1);
  }

  BeatManager* beatManager;
  PaletteManager* paletteManager;
  ColorSequencer* colorSequencer;
  AnimationSequencer* animSequencer;
};

class Animation {
public:
  virtual void Draw( float beat, int* paletteChoices, Palette* palette ) = 0;  
};

class SimpleBeat : public Animation {
public:
  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    trellis.fill(0);
    trellis.setPixelColor( (int)beat%32, palette->Color( paletteChoices[(int)beat%4] ) );    
  }
};

class FadePixel
{
public:
  FadePixel() : pixel(0), running(false), r(0), g(0), b(0), duration(0) {}  
  void SetPixel( int i ) {
    pixel = i;
  }
  
  bool Running() { return running; }
  
  void Start( uint32_t color, float startTime, float duration ) {
    if( running ) return;
    running = true;
    r = ((color & 0x00FF0000)>>16);
    g = ((color & 0x0000FF00)>>8);
    b = ((color & 0x000000FF));
    this->duration = duration;
    this->startTime = startTime;  
  }

  void Update( float currentBeat ) {
    if( currentBeat >= startTime + duration ) {
      running = false;
    }
    if( !running ) return;
    float t = 1.0f-((currentBeat-startTime)/duration);
    trellis.setPixelColor( pixel, trellis.Color( t*r, t*g, t*b ));
  }

private:
  int pixel;
  byte r,g,b;
  bool running;
  float startTime;
  float duration;
};

class ChaseBeat : public Animation {
public:
  ChaseBeat() { 
  }

  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    
  }
};

class SquareBeat : public Animation {
public:
  SquareBeat() {
    for( int i = 0; i < 4; ++i ) colors[i] = 0;
  }

  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    if( lastBeat != (int)beat ) {
      lastBeat = beat;
      colors[1] = colors[0];
      colors[3] = colors[2];
      int i = beat;
      colors[0] = palette->Color(paletteChoices[(int)i%4]);
      colors[2] = palette->Color(paletteChoices[(int)(i+2)%4]);
    }
    
    DrawButton(0,0,colors[1],4,4);
    DrawButton(1,1,colors[0],2,2);
    DrawButton(4,0,colors[3],4,4);
    DrawButton(5,1,colors[2],2,2);
  }

private:
  int lastBeat;
  uint32_t colors[4];
};

class DiscoBeat : public Animation {
public:
  DiscoBeat() {
    for( int i = 0; i < 32; ++i ) {
      pixels[i].SetPixel(i);
    }
  }

  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    if( lastBeat != (int)beat ) {
      lastBeat = beat;
        int i = 0;
        for( int j = 0; j < 2; ++j ) {
          uint32_t col = palette->Color(paletteChoices[rand()%4]);
          while( ++i < 32 ) { 
            int px = (rand()%4)*8 + (rand()%4);
            if( pixels[px].Running() ) continue; 
            pixels[px].Start( col, beat, 4.0f );
            break;
          }
          while( ++i < 32 ) { 
            int px = (rand()%4)*8 + (rand()%4) + 4;
            if( pixels[px].Running() ) continue; 
            pixels[px].Start( col, beat, 4.0f );
            break;
          }
        }
    }

    trellis.fill(0);
    for( int i = 0; i < 32; ++i ) {
      pixels[i].Update(beat);
    }
  }

private:
  int lastBeat;

  FadePixel pixels[32];
};

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
