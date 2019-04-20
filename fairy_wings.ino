#include "Adafruit_NeoTrellisM4.h"

// The NeoTrellisM4 object is a keypad and neopixel strip subclass
// that does things like auto-update the NeoPixels and stuff!
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();
int brightness = 20;

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
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
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
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
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
  
private:
  int paletteChoices[4][4];
  Palette* palette;
  PaletteChooser* chooser;
  int selectedCol;
  int selectedPal;
};

class BeatManager {
public:
  BeatManager() : bpm(30.0f) {
    nextBeatAt = TimeBetweenBeats();
  }
  
  void Tick() {
    if( millis() >= nextBeatAt ) {
      ++activeBeat;
      activeBeat %= 32;
      nextBeatAt += TimeBetweenBeats();
    }
  }

  int ActiveBeat() { return activeBeat; }
  void BPM(float value) { bpm = value; }
  
private:
  unsigned long nextBeatAt;
  int activeBeat;
  float bpm;
  
  unsigned long TimeBetweenBeats() {
    return (unsigned long)((1.0f/(bpm/60.0f))*1000.0f);
  }  
};

class ColorSequencer : public UIInterface {
public:
  ColorSequencer( BeatManager* beatManager ) : beatManager(beatManager), lastBeat(0) {}
  
  void Setup() {
    color[0] = trellis.Color(255,0,0);
    color[1] = trellis.Color(0,255,0);
    color[2] = trellis.Color(0,0,255);
    color[3] = trellis.Color(255,255,0);        

    for( int i = 0; i < MAX_BEATS; ++i ) {
      paletteChoice[i] = 0;
    }
    updateDisplay();
  }
  
  void Shutdown() {
    
  }

  void OnKeyEvent( keypadEvent e ) {
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
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
  AnimationSequencer( BeatManager* beatManager ) : beatManager(beatManager) {}
  
  void Setup() {
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
    UpdateDisplay();
  }
  
  void Shutdown() {
   
  }
  
  void OnKeyEvent( keypadEvent e ) {
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
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

UIManager uiManager;
RainbowPalette palette;
PaletteManager* paletteManager;
ColorSequencer* colorSequencer;
AnimationSequencer* animSequencer;
BeatManager beatManager;

void setup(){
  trellis.begin();
  trellis.autoUpdateNeoPixels(false);
  trellis.setBrightness(brightness);

  palette.Size(trellis.num_keys());
  paletteManager = new PaletteManager(&palette);
  colorSequencer = new ColorSequencer(&beatManager);
  animSequencer = new AnimationSequencer(&beatManager);
  
  uiManager.push(animSequencer);
}
  
void loop() {
  // put your main code here, to run repeatedly:
  trellis.tick();
  beatManager.Tick();
  
  while (trellis.available()){
    keypadEvent e = trellis.read();
    uiManager.active_menu()->OnKeyEvent(e);
  }

  uiManager.active_menu()->Tick();
  trellis.show();
  delay(10);
}
