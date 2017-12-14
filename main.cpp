#include <vector>
#include <string>
#include <iostream>

#include <windows.h>

#include "dir_monitor.h"

typedef void (*FuncAeUpdate)( );
FuncAeUpdate aeUpdate = 0;
HMODULE GameDll;

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
  aeUpdate = 0;
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
    
    aeUpdate = (FuncAeUpdate)GetProcAddress(GameDll, "aeUpdate");
    if(!aeUpdate)
    {
      DWORD err = GetLastError();
      std::cout << "Failed to load update func: " << err << std::endl;
      return false;
    }
  }
  
  return true;
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
    
    LoadGameDll();
    
    DirMon dirMon;
    dirMon.Init();
    
    while(true)
    {
      dirMon.Poll();
      aeUpdate();
    }
    
    std::getchar();
    
    return EXIT_SUCCESS;
}