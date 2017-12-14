#include <iostream>
#include "../aurora.h"


class Gameplay : public GameState
{
public:
  void Update()
  {
    std::cout << "test" << std::endl;
  }
  
  void Cleanup()
  {
    delete this;
  }
};

GameState* aeInit()
{
    return new Gameplay();
}

void aeUpdate()
{
  std::cout << "qwe" << std::endl;
}