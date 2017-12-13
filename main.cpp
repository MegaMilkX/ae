#include <vector>
#include <string>
#include <iostream>

#include <windows.h>

template<typename T>
void print_vector(const std::vector<T>& data)
{
    for(auto& d : data)
    {
        std::cout << d << std::endl;
    }
}

HMODULE ThisModuleHandle()
{
  HMODULE h = NULL;
  GetModuleHandleExW(
    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    reinterpret_cast<LPCWSTR>(&ThisModuleHandle),
    &h
  );
  return h;
}

std::string ThisModuleDir()
{
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(ThisModuleHandle(), buf, MAX_PATH);
    std::string dir(buf, buf + len);
    return dir.substr(0, dir.find_last_of('\\'));
}

HANDLE hDir;
std::vector<BYTE> buffer;

VOID CALLBACK ChangeCallback(DWORD, DWORD, LPOVERLAPPED);

void ReadDirChanges(HANDLE hDir)
{
    OVERLAPPED overlapped;
    overlapped.hEvent = NULL;
    
    DWORD dwBytes = 0;
    if(!ReadDirectoryChangesW(
        hDir,
        &buffer[0],
        buffer.size(),
        FALSE,
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        &dwBytes,
        &overlapped,
        &ChangeCallback
    ))
    {
        std::cout << GetLastError();
    }
    
    SleepEx(INFINITE, TRUE);
}

VOID CALLBACK ChangeCallback(
	DWORD dwErrorCode,									// completion code
	DWORD dwNumberOfBytesTransfered,					// number of bytes transferred
	LPOVERLAPPED lpOverlapped)				
{
    if(!dwNumberOfBytesTransfered)
		return;
    
    BYTE* pBase = buffer.data();

	for (;;)
	{
		FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*pBase;
        if(fni.Action == FILE_ACTION_MODIFIED)
        {
            std::wcout << std::wstring(fni.FileName, fni.FileName + fni.FileNameLength) << std::endl;
        }
        if (!fni.NextEntryOffset)
			break;
		pBase += fni.NextEntryOffset;        
    }
    
    ReadDirChanges(hDir);
}

int main(int argc, char** argv)
{
    print_vector(std::vector<char*>(argv, argv + argc));
    
    buffer.resize(sizeof(FILE_NOTIFY_INFORMATION) * 10);
    
    CreateDirectoryA("data", NULL);
    
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
		return EXIT_FAILURE;
	}
    
    ReadDirChanges(hDir);    
    std::getchar();
    
    return EXIT_SUCCESS;
}