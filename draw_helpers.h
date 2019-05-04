#ifndef __DRAW_HELPERS_H__
#define __DRAW_HELPERS_H__

#include "trellis.h"

void DrawButton( int x, int y, uint32_t color, int w = 2, int h = 2 )
{
  for( int cx = x; cx < x + w; ++cx ) {
    for( int cy = y; cy < y + h; ++cy ) {
      trellis.setPixelColor((cy*8)+cx, color);
    }
  }       
}

#endif
