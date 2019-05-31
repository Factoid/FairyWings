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

class RainbowSwirl : public Animation {
public:
  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    int offset = (int)beat%32*255/32;
    for( int i = 0; i < 32; ++i ) {
      trellis.setPixelColor( i, RainbowPalette::Wheel( (int)(offset + (i*255/31))%255) );
    }
  }  
};

class PrideBeat : public Animation {
public:
  PrideBeat() {    
    prideColor[0] = trellis.Color(255,0,0);
    prideColor[1] = trellis.Color(255,64,0);
    prideColor[2] = trellis.Color(255,255,0);
    prideColor[3] = trellis.Color(0,255,0);
    prideColor[4] = trellis.Color(0,0,255);
    prideColor[5] = trellis.Color(128,0,64);

    const int c = 110;
    transFlag[0] = trellis.Color(c,c,255);
    transFlag[1] = trellis.Color(255,c,c);
    transFlag[2] = trellis.Color(255,255,255);

    panFlag[0] = trellis.Color(255,0x14,64);
    panFlag[1] = trellis.Color(255,0xDA,00);
    panFlag[2] = trellis.Color(5,0xAE,0xFF);

    curFlag = 0;
  }
  
  void Draw( float beat, int* paletteChoices, Palette* palette ) {
    if( (int)beat != lastBeat ) {
      lastBeat = (int)beat;
      int i = (lastBeat+9)%8;
      if( i == 0 ) {
        switch( curFlag ) {
          case 0:
            MakePride();
            break;
          case 1:
            MakeTrans();
            break;
          case 2:
            MakePan();
        }
        curFlag += 1;
        curFlag %= 3;
      }
      trailPixel[i].Start( beat, 3, i, 8, colors[i] );
    }

    mixBuffer.FillBuffer(0,0,0);
    for( int i = 0; i < 8; ++i ) {
      trailPixel[i].Tick(beat,mixBuffer);
    }
/*    
    int y = lastBeat%4;
    bool left = (lastBeat%8) < 4;
    mixBuffer.AddBuffer( (y*8)+(left ? 0 : 7), 255, 255, 255 );
*/
/*
    uint32_t c = RainbowPalette::Wheel((int)(beat*64)%255);
    int r = ((c & 0x00FF0000)>>16) / 4;
    int g = ((c & 0x0000FF00)>>8) / 4;
    int b = ((c & 0x000000FF)) / 4;
    for( int i = 0; i < 4; ++ i ) {
      mixBuffer.AddBuffer( (i*8), r, g, b );
      mixBuffer.AddBuffer( (i*8)+7, r, g, b );
    }
*/
    mixBuffer.ShowBuffer();
  }
  
private:
  uint32_t getColor( float beat ) {
    return prideColor[(int)beat%6];
  }

  void MakePride() {
    colors[0] = colors[7] = 0;
    for( int i = 0; i < 6; ++i ) {
      colors[i+1] = prideColor[i];
    }
  }

  void MakeTrans() {
    colors[0] = colors[7] = 0;
    colors[1] = colors[6] = transFlag[0];
    colors[2] = colors[5] = transFlag[1];
    colors[3] = colors[4] = transFlag[2];
  }

  void MakePan() {
    colors[0] = colors[7] = 0;
    colors[1] = colors[2] = panFlag[0];
    colors[3] = colors[4] = panFlag[1];
    colors[5] = colors[6] = panFlag[2];
  }
  
  MixBuffer mixBuffer;
  TrailPixel trailPixel[8];
  uint32_t colors[8];
  uint32_t prideColor[6];
  uint32_t transFlag[3];
  uint32_t panFlag[3];
    
  int lastBeat;
  int curFlag;
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
