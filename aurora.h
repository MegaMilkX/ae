#ifndef AURORA_ENGINE_H
#define AURORA_ENGINE_H

class GameState
{
public:
  virtual void Update() = 0;
  virtual void Cleanup() = 0;
};

#define DLLEXPORT __declspec(dllexport)
extern "C"
{
DLLEXPORT GameState* aeInit(void);
DLLEXPORT void aeUpdate(void);
}

#endif
