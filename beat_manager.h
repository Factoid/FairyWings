#ifndef __BEAT_MANAGER_H__
#define __BEAT_MANAGER_H__

#include "trellis.h"

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

#endif
