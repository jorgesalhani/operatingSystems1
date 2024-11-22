#include <windows.h>
#include <stdio.h>
#include <tchar.h>

int main(int argc, TCHAR *argv[]) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (argc != 2) {
    printf("Usage: %s [cmdline]\n", argv[0]);
    return 0;
  }

  if (!CreateProcess(
    NULL,
    argv[1],
    NULL,
    NULL,
    FALSE,
    0,
    NULL,
    NULL,
    &si,
    &pi
  )) {
    printf("CreateProcess failed (%d).\n", GetLastError());
    return -1;
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return 0;
}
