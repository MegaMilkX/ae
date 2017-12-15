#include <vector>
#include <string>
#include <iostream>

#include <windows.h>

#include "dir_monitor.h"

#include "aurora.h"

class Core : public AuroraCore
{
public:
  Core()
   : cur(0)
  {
    mem_block.resize(50000000);
  }
  virtual void* Malloc(size_t n)
  {
    void* p = mem_block.data() + cur;
    cur += n;
    return p;
  }
  virtual void Free(void* p)
  {
    
  }
private:
  std::vector<unsigned char> mem_block;
  size_t cur;
};

typedef GameState*(*FuncAeInit)(AuroraCore*);
typedef void (*FuncAeUpdate)(GameState*);
FuncAeInit _aeInit = 0;
FuncAeUpdate _aeUpdate = 0;
HMODULE GameDll;
GameState* state;
Core* core;

template<typename T>
void print_vector(const std::vector<T>& data)
{
    for(auto& d : data)
    {
        std::cout << d << std::endl;
    }
}

void UnloadGameDll()
{
  FreeLibrary(GameDll);
  GameDll = 0;
  _aeInit = 0;
  _aeUpdate = 0;
}

bool LoadGameDll()
{
  WIN32_FILE_ATTRIBUTE_DATA unused;
  if(!GetFileAttributesExA("lock.tmp", GetFileExInfoStandard, &unused))
  {
    UnloadGameDll();
    CopyFileA("game.dll", "game_temp.dll", 0);
    GameDll = LoadLibraryA("game_temp.dll");
    
    if(!GameDll)
    {
      DWORD err = GetLastError();
      std::cout << "LoadLibraryA failed: " << err << std::endl;
      return false;
    }
    
    _aeInit = (FuncAeInit)GetProcAddress(GameDll, "aeInit");
    _aeUpdate = (FuncAeUpdate)GetProcAddress(GameDll, "aeUpdate");
    if(!_aeUpdate || !_aeInit)
    {
      DWORD err = GetLastError();
      std::cout << "Failed to load functions: " << err << std::endl;
      return false;
    }
    
    state = _aeInit(core);
    return true;
  }
  
  return false;
}

class DirMon : public DirMonitor
{
public:
  void OnModify(const std::string& filename)
  {
    if(filename == "game.dll")
    {
      if(!LoadGameDll())
      {
        std::cout << "Failed to reload dll" << std::endl;
        std::getchar();
      }
      std::cout << filename << std::endl;
    }
  }
};

int main(int argc, char** argv)
{
    print_vector(std::vector<char*>(argv, argv + argc));
    
    core = new Core();
    LoadGameDll();
    
    DirMon dirMon;
    dirMon.Init();
    
    while(true)
    {
      dirMon.Poll();
      state->Update();
      //_aeUpdate(state);
    }
    
    std::getchar();
    
    return EXIT_SUCCESS;
}