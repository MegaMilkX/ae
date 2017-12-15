#include <iostream>
#include <stdlib.h>
#include "../aurora.h"

AuroraCore* core;

class Gameplay : public GameState
{
public:
  virtual ~Gameplay() 
  {
    
  }
  void Update()
  {
    std::cout << "uiu7uiiu" << std::endl;
  }
  
  void Cleanup()
  {
    delete this;
  }
};



void* operator new(size_t n)
{
  if(!core)
  {
    return malloc(n);
  }
  else
  {
    return core->Malloc(n);
  }
}

void operator delete(void* p)
{
  if(!core)
    free(p);
  else
    core->Free(p);
}

GameState* aeInit(AuroraCore* c)
{
  core = c;
  return new Gameplay();
}

void aeUpdate(GameState* state)
{
  state->Update();
}