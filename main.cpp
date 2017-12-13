#include <vector>
#include <string>
#include <iostream>

#include <windows.h>

#include "dir_monitor.h"

template<typename T>
void print_vector(const std::vector<T>& data)
{
    for(auto& d : data)
    {
        std::cout << d << std::endl;
    }
}

class DirMon : public DirMonitor
{
public:
  void OnModify(const std::string& filename)
  {
    std::cout << filename << " modified" << std::endl;
  }
};

int main(int argc, char** argv)
{
    print_vector(std::vector<char*>(argv, argv + argc));
    
    DirMon dirMon;
    dirMon.Init();
    
    while(true)
    {
      dirMon.Poll();
    }
    
    std::getchar();
    
    return EXIT_SUCCESS;
}