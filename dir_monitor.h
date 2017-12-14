#ifndef DIR_MONITOR_H
#define DIR_MONITOR_H

#include <mutex>
#include <queue>
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>

inline HMODULE ThisModuleHandle()
{
  HMODULE h = NULL;
  GetModuleHandleExW(
    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    reinterpret_cast<LPCWSTR>(&ThisModuleHandle),
    &h
  );
  return h;
}

inline std::string ThisModuleDir()
{
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(ThisModuleHandle(), buf, MAX_PATH);
    std::string dir(buf, buf + len);
    return dir.substr(0, dir.find_last_of('\\')) + "\\";
}

class DirMonitor
{
public:
  enum MsgType
  {
      MODIFY
  };

  struct Msg
  {
    Msg() {}
    Msg(MsgType type, const std::string& filename)
    : type(type), filename(filename)
    {
    }
    MsgType type;
    std::string filename;
  };

  virtual void OnModify(const std::string& filename) = 0;

  DWORD threadId;
  HANDLE thread;
  bool Init()
  {
    if(!InitImpl())
      return false;
    
    thread = CreateThread(
      NULL,
      0,
      &DirMonitor::DirMonitorThread,
      this,
      0,
      &threadId
    );
    
    return true;
  }
  
  bool InitImpl()
  {
    buffer.resize(sizeof(FILE_NOTIFY_INFORMATION) * 10);
    
    hDir = CreateFileA(
        ThisModuleDir().c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );
    
    if (hDir == INVALID_HANDLE_VALUE)
    {
      std::cout << "Failed to get dir handle" << std::endl;
      std::getchar();
      return false;
    }
    
    return true;
  }
  void Cleanup()
  {
    
  }
  
  void Poll()
  {
    while(true)
    {
      sync_queue.lock();
      Msg m;
      if(!queue.empty())
      {
        m = queue.front();
        queue.pop();
      }
      else
      {
        sync_queue.unlock();
        break;
      }
      sync_queue.unlock();
      
      if(m.type == MODIFY)
      {
          OnModify(m.filename);
      }
    }
  }
  
private:
  void Run()
  {
    ZeroMemory(&overlapped, sizeof(OVERLAPPED));
    overlapped.hEvent = this;
    
    DWORD dwBytes = 0;
    if(!ReadDirectoryChangesW(
        hDir,
        &buffer[0],
        buffer.size(),
        FALSE,
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        &dwBytes,
        &overlapped,
        &DirMonitor::ChangeCallback
    ))
    {
        std::cout << GetLastError();
    }
    
    SleepEx(INFINITE, TRUE);
  }

  static VOID CALLBACK ChangeCallback(DWORD errCode, DWORD numBytes, LPOVERLAPPED lpOverlapped)
  {
    if(!numBytes)
      return;
    
    DirMonitor* mon = (DirMonitor*)lpOverlapped->hEvent;
    
    BYTE* pBase = mon->buffer.data();
    
    while(true)
    {
      FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*pBase;
      if(fni.Action == FILE_ACTION_MODIFIED)
      {
        mon->sync_queue.lock();
        std::wstring wstr(fni.FileName, fni.FileNameLength / sizeof(wchar_t));
        mon->queue.push(Msg(MODIFY, std::string(wstr.begin(), wstr.end())));
        mon->sync_queue.unlock();
      }
      if (!fni.NextEntryOffset)
        break;
      pBase += fni.NextEntryOffset;
    }
    
    mon->Run();
  }
  
  static DWORD __stdcall DirMonitorThread(LPVOID lpThreadParameter)
  {
    DirMonitor* mon = (DirMonitor*)lpThreadParameter;
      
    mon->Run();
      
    return 0;
  }

  std::mutex sync_queue;
  std::queue<Msg> queue;
  HANDLE hDir;
  std::vector<BYTE> buffer;
  OVERLAPPED overlapped;
};



#endif
