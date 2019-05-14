#ifndef __LIFE_GRID_H__
#define __LIFE_GRID_H__

class LifeGrid {
public:
  LifeGrid() {
    Clear();
  }

  void Spawn( int x, int y ) {
    if( !Alive(x,y) ) {
      cells[(y*8)+x] = CellState::spawned;
      ++liveCount;  
    }
  }

  uint32_t FadeColor( int x, int y, float t ) {
    uint32_t color = colors[(y*8)+x];
    int r = ((color & 0x00FF0000)>>16);
    int g = ((color & 0x0000FF00)>>8);
    int b = ((color & 0x000000FF));
    return trellis.Color( t*r, t*g, t*b );
  }

  uint32_t Color( int x, int y ) {
    return colors[(y*8)+x];
  }
  
  void Next(int* paletteChoices, Palette* palette) {
    CellState newState[32];
    for( int x = 0; x < 8; ++x ) {
      for( int y = 0; y < 4; ++y ) {
        int i = (y*8)+x;
        int n = Neighbours(x,y);
        if( Alive(x,y) ){
          if( n == 2 || n == 3 ) {
            newState[i] = CellState::alive;          
          } else {
            newState[i] = CellState::dying;
            --liveCount;
          }
        } else {
          if( n == 3 ) {
            newState[i] = CellState::spawned;
            colors[i] = palette->Color(paletteChoices[rand()%4]);
            ++liveCount;
          } else {
            newState[i] = CellState::dead;
          }
        }
      }
    }
    for( int i = 0; i < 32; ++i ) {
      cells[i] = newState[i];
    }
  }

  bool FullAlive( int x, int y ) {
    return cells[(y*8)+x] == CellState::alive;
  }

  bool Spawning( int x, int y ) { 
    return cells[(y*8)+x] == CellState::spawned;
  }

  bool Dying( int x, int y ) {
    return cells[(y*8)+x] == CellState::dying; 
  }
  
  bool Alive( int x, int y ) {
    if( x > 7 ) x -= 8;
    if( x < 0 ) x += 8;
    if( y < 0 ) y += 4;
    if( y > 4 ) y -= 4;
    
    int i = (y*8) + x;
    return Alive(i);
  }

  void SpawnMinCells(int* paletteChoices, Palette* palette) {
    while( liveCount < 8 ) {
      int i = rand()%32;
      if( !Alive(i) ) {
        cells[i] = CellState::spawned;
        colors[i] = palette->Color(paletteChoices[rand()%4]);
        ++liveCount;
      }
    }
    while( liveCount > 24 ) {
      int i = rand()%32;
      if( Alive(i) ) {
        cells[i] = CellState::dying;
        --liveCount;
      }
    }
  }

  void Clear() {
    for( int i = 0; i < 32; ++i ) cells[i] = CellState::dead;
    liveCount = 0;
  }

  void Mutate( int nCells ) {
    for( int i = 0; i < nCells; ) {
      int ndx = rand()%32;
      switch( cells[ndx] ) {
        case CellState::dead:
        cells[ndx] = CellState::spawned;
        ++liveCount;
        ++i;
        break;
        case CellState::alive:
        cells[ndx] = CellState::dying;
        --liveCount;
        ++i;
        break;
      }
    }
  }
    
private:
  int liveCount;
  enum CellState { dead, spawned, alive, dying };
  CellState cells[32];
  uint32_t colors[32];

  bool Alive( int i ) {
    return cells[i] == CellState::alive || cells[i] == CellState::spawned;        
  }
  
  int Neighbours( int x, int y ) {
    int n = 0;
    
    for( int dx = x-1; dx <= x+1; ++dx ) {
      for( int dy = y-1; dy <= y+1; ++dy ) {
        if( dx == x && dy == y ) continue;
        if( Alive(dx,dy) ) {
          ++n;
        }
      }
    }
    return n;
  }
};

#endif
