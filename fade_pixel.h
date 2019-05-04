#ifndef __FADE_PIXEL_H__
#define __FADE_PIXEL_H__

#include "trellis.h"

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

#endif
