#ifndef __TRAIL_PIXEL_H__
#define __TRAIL_PIXEL_H__

#include "trellis.h"

class TrailPixel
{
public:
  TrailPixel() : isRunning(false), r(0), g(0), b(0), dir(0), velocity(0.1f) {    
  }
  
  void Start( int y, uint32_t color ) {
    if( isRunning ) return;
    isRunning = true;
    r = ((color & 0x00FF0000)>>16);
    g = ((color & 0x0000FF00)>>8);
    b = ((color & 0x000000FF));
    x = 0;
    this->y = y;
    velocity = (rand()%2) == 0 ? 0.05f : 0.15f;
  }
  
  bool IsRunning() { return isRunning; }

  void Tick( MixBuffer& mixBuffer ) {
    if( !isRunning ) return;
    
    float t = 1.0 - ( x - (int)x );
//    SetPixel( (int)x, (int)y, t );
//    SetPixel( (int)x+1, (int)y, 1.0-t );

    SetPixel( mixBuffer, (int)x, (int)y, 1.0f );
    SetPixel( mixBuffer, (int)x-1, (int)y, 0.75f );
    SetPixel( mixBuffer, (int)x-2, (int)y, 0.5f );
    SetPixel( mixBuffer, (int)x-3, (int)y, 0.25f );

    x += velocity;
    if( x-4 >= 8 ) isRunning = false;
  }
  
private:
  bool isRunning;
  float x, y;
  int dir;
  float velocity;
  byte r,g,b;

  void SetPixel( MixBuffer& mixBuffer, int x, int y, float t ) {
    if( x > 7 || x < 0 ) return;
    if( y > 4 || y < 0 ) return;
    mixBuffer.AddBuffer( (y*8)+x, t*r, t*g, t*b );    
  }
};

#endif
