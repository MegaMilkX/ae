#ifndef AURORA_ENGINE_H
#define AURORA_ENGINE_H

class AuroraCore
{
public:
  virtual void* Malloc(size_t) = 0;
  virtual void Free(void*) = 0;
};

class GameState
{
public:
  virtual ~GameState() {}
  virtual void Update() = 0;
  virtual void Cleanup() = 0;
};

#define DLLEXPORT __declspec(dllexport)
extern "C"
{
DLLEXPORT GameState* aeInit(AuroraCore*);
DLLEXPORT void aeUpdate(GameState*);
}

#endif
