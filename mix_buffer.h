#ifndef __MIX_BUFFER_H__
#define __MIX_BUFFER_H__

#include "trellis.h"

class MixBuffer {
public:
  void FillBuffer( int r, int g, int b ) {
    for( int i = 0; i < 32; ++i ) {
      rBuf[i] = r;
      gBuf[i] = g;
      bBuf[i] = b;
    }
  }

  void AddBuffer( int px, int r, int g, int b ) {
    rBuf[px] += r;
    gBuf[px] += g;
    bBuf[px] += b;
  }

  void ShowBuffer() {
    for( int i = 0; i < 32; ++i ){
      trellis.setPixelColor(i, trellis.Color( min(rBuf[i],255), min(gBuf[i],255), min(bBuf[i],255) ) );
    }
  }

private:
  int rBuf[32], gBuf[32], bBuf[32];  
};

#endif
