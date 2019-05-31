#ifndef __TRAIL_PIXEL_H__
#define __TRAIL_PIXEL_H__

#include "trellis.h"

class TrailPixel
{
public:
  TrailPixel() : isRunning(false) {    
  }

  void Start( float beat, int d, int p, int s, uint32_t color ) {
    if( isRunning ) return;
    isRunning = true;
    startBeat = beat;
    r = ((color & 0x00FF0000)>>16);
    g = ((color & 0x0000FF00)>>8);
    b = ((color & 0x000000FF));
    spd = s;
    dir = d;
    switch( dir ) {
      case 0:
        sx = 0;
        ex = 7;
        sy = p;
        ey = sy;
        break;
      case 1:
        sx = 7;
        ex = 0;
        sy = p;
        ey = sy;        
        //sy = ey = 3;
        break;
      case 2:
        sx = p;
        ex = sx;
        sy = 0;
        ey = 3;
        break;
      case 3:
        sx = p;
        ex = sx;
        sy = 4;
        ey = 0;
        break;
    }    
  }

  void Start( float beat, int d, uint32_t color ) {
    int s = 1 << (rand()%3);
    switch( d ) {
      case 0:
      case 1:
        Start( beat, d, rand() % 4, s, color );
        break;
      case 2:
      case 3:
        Start( beat, d, rand() % 8, s, color );
        break;
    }
  }
  
  bool IsRunning() { return isRunning; }

  void Tick( float beat, MixBuffer& mixBuffer ) {
    if( beat > startBeat + (8/spd) + 4 ) isRunning = false;
    if( !isRunning ) return;
    float n = beat-startBeat;
    int x, y;
    switch( dir ) {
      case 0:
      x = 0 + (int)(n*spd);
      y = sy;
      break;
      case 1:
      x = 7 - (int)(n*spd);
      y = sy;
      break;
      case 2:
      y = 0 + (int)(n/2*spd);
      x = sx;
      break;
      case 3:
      y = 3 - (int)(n/2*spd);
      x = sx;
      break;
    }
//    SetPixel( mixBuffer, x, y, 1.0f );

    for( int i = 1; i <= 4; ++i ) {
      SetPixel( mixBuffer, x, y, 1.0f/i );
      switch( dir ) {
        case 0:
        x -= 1;
        break;
        case 1:
        x += 1;
        break;
        case 2:
        y -= 1;
        break;
        case 3:
        y += 1;
        break;
      }
    }

//    mixBuffer.AddBuffer( (int)n, 255, 255, 255 ); 
  }
  
private:
  bool isRunning;
  int sx, sy;
  int ex, ey;
  int spd;
  float startBeat;
  int dir;
  byte r,g,b;

  void SetPixel( MixBuffer& mixBuffer, int x, int y, float t ) {
    if( x > 7 || x < 0 ) return;
    if( y > 3 || y < 0 ) return;
    mixBuffer.AddBuffer( (y*8)+x, t*r, t*g, t*b );    
  }
};

#endif
