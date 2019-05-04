#ifndef __UI_INTERFACE_H__
#define __UI_INTERFACE_H__

#include "trellis.h"

class UIManager;

class UIInterface {
public:
  virtual void Setup() = 0;
  virtual void Shutdown() = 0;
  virtual void OnKeyEvent( keypadEvent e ) = 0;
  virtual void Tick() = 0;

  static void SetManager( UIManager* manager ) { UIInterface::manager = manager; }
protected:
  static UIManager* manager;
};

UIManager* UIInterface::manager = 0;

class UIManager {
public:
  UIManager() { UIInterface::SetManager(this); }
  UIInterface* active_menu() { 
    if( activeInterface < 0 ) return NULL;
    return stack[activeInterface];
  }
  
  void push( UIInterface* interface ) {
    if( activeInterface >= maxStack - 1 ) return;
    if( activeInterface >= 0 ) stack[activeInterface]->Shutdown();
    ++activeInterface;
    stack[activeInterface] = interface;
    stack[activeInterface]->Setup();
  }
  
  void pop() {
    stack[activeInterface]->Shutdown();
    stack[activeInterface] = NULL;
    --activeInterface;
    if( activeInterface >= 0 ) stack[activeInterface]->Setup();
  }

  int numUI() {
    return activeInterface + 1;
  }
  
private:
  static const int maxStack = 12;
  UIInterface* stack[maxStack];
  int activeInterface = -1;
};

#endif
