#ifndef __ANIMATIONS_H__
#define __ANIMATIONS_H__
#include "draw_helpers.h"
#include "mix_buffer.h"
#include "trail_pixel.h"
#include "fade_pixel.h"
#include "trellis.h"

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

class GridBeat : public Animation {
  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    for( int x = 0; x < 4; ++x ) {
      for( int y = 0; y < 2; ++y ) {
        trellis.setPixelColor( (y*16) + (x*2), palette->Color(paletteChoices[(int)beat%4]) );
        trellis.setPixelColor( (y*16) + (x*2) + 1, palette->Color(paletteChoices[(int)(beat+1)%4]) );
        trellis.setPixelColor( (y*16) + (x*2) + 9, palette->Color(paletteChoices[(int)(beat+2)%4]) );
        trellis.setPixelColor( (y*16) + (x*2) + 8, palette->Color(paletteChoices[(int)(beat+3)%4]) );
      }
    }
  }
};

class ChaseBeat : public Animation {
public:
  ChaseBeat() : lastBeat(0) { 
  }

  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    if( beat > lastBeat + 1.0f ) {
      lastBeat = beat;
      for( int i = 0; i < 32; ++i ) {
        if( !pixels[i].IsRunning() ) {
          pixels[i].Start( rand()%4, palette->Color(paletteChoices[rand()%4]));
          break;    
        }
      }
    }
    mixBuffer.FillBuffer(0,0,0);
    for( int i = 0; i < 32; ++i ) {
      pixels[i].Tick(mixBuffer);
    }
    mixBuffer.ShowBuffer();
  }

private:
  MixBuffer mixBuffer;
  TrailPixel pixels[32];
  float lastBeat;
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

#endif
