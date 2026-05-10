# Lazarus
## Problem:

CPU and Graphics card can't work with large process cause their work with many process.

## Solution:

Lazarus solve this problem, it freeze trash process and give all resources to main process.

## Under the hood:

Lazarus use Windows API to control threads:


```cpp   
    HANDLE hThreat = OpenThread(THREAD_SUSPEND_RESUME, false, te32.th32ThreadID);
    if (hThreat != NULL) {
     if (freeze == true) {
      SuspendThread(hThreat);
```
**And force memory:


```cpp
OpenProcess(PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION, false, targetPID);
if (hProcess != NULL)
{SetProcessWorkingSetSize(hProcess, (SIZE_T)-1, (SIZE_T)-1);
}
```
# Instruction
## Control:
Add bad and trigger process in special lists. When you open trigger process bad is freeze.You also can add process in both lists.

To start work programm press F8 or button *'Start'*

To stop work programm press F10 or button *'Stop'*

# If something wrong
## Make sure:
1. In folder with Lazarus.exe lies *BlackList.txt* and *Trigger.txt*
2. *BlackList.txt* and *Trigger.txt* save in encoding *ANSI* or *UTF - 16 LE* prefer *ANSI*
## Contacts
[![Telegram](https://img.shields.io/badge/Telegram-26A5E4?style=for-the-badge&logo=telegram&logoColor=white)](https://t.me/@nofififi)
