#ifndef __ANIMATIONS_H__
#define __ANIMATIONS_H__
#include "draw_helpers.h"
#include "mix_buffer.h"
#include "trail_pixel.h"
#include "fade_pixel.h"
#include "life_grid.h"
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

class LifeBeat : public Animation {
public:
  LifeBeat() : lastBeat(0) {
  }
  
  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    if( (int)beat != lastBeat ) {
      lastBeat = (int)beat;
      if( lastBeat % 8 == 0 ) {
        lifeGrid.Mutate(2);
      }
      lifeGrid.Next(paletteChoices,palette);
      lifeGrid.SpawnMinCells(paletteChoices,palette);
    }

    trellis.fill(0);
    for( int x = 0; x < 8; ++x ){
      for( int y = 0; y < 4; ++y ) {
        float v = (beat - (int)beat)*4.0f;
        if( v > 1 ) v = 1.0f;
        if( lifeGrid.FullAlive(x,y) ) trellis.setPixelColor( (y*8) + x, lifeGrid.Color(x,y));
        if( lifeGrid.Spawning(x,y) ) trellis.setPixelColor( (y*8) + x, lifeGrid.FadeColor( x, y, v ));
        if( lifeGrid.Dying(x,y) ) trellis.setPixelColor( (y*8) + x, lifeGrid.FadeColor(x,y, 1.0f-v ));
      }
    }
  }
private:
  int lastBeat;
  LifeGrid lifeGrid;
};

class ChaseBeat : public Animation {
public:
  ChaseBeat() : lastBeat(-1), ci(0) { 
  }

  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    int b = (int)beat;
    if( b != lastBeat ) {
      lastBeat = b;
      for( int i = 0; i < 32; ++i ) {
        if( !pixels[i].IsRunning() ) {
          pixels[i].Start(beat,rand()%4, palette->Color(paletteChoices[ci]));
          ci += 1;
          ci %= 4;
          break;    
        }
      }
    }
    mixBuffer.FillBuffer(0,0,0);
    for( int i = 0; i < 32; ++i ) {
      pixels[i].Tick(beat,mixBuffer);
    }
    mixBuffer.ShowBuffer();
  }

private:
  int ci;
  MixBuffer mixBuffer;
  TrailPixel pixels[32];
  int lastBeat;
};

class SquareBeat : public Animation {
public:
  SquareBeat() {
    for( int i = 0; i < 4; ++i ) colors[i] = 0;
  }

  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    if( lastBeat != (int)beat ) {
      lastBeat = beat;
      // 0 -> 1 -> 3 -> 2;
      int n = (int)beat%4;
      colors[2] = colors[3];
      colors[3] = colors[1];
      colors[1] = colors[0];
      colors[0] = palette->Color(paletteChoices[n%4]);
      ++n;
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
