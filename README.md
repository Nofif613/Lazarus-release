<img width="731" height="642" alt="preview" src="https://github.com/user-attachments/assets/6cee3470-66d6-41e0-b64a-ec3fc9cf6127" />


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
## Before
<img width="1055" height="433" alt="Before" src="https://github.com/user-attachments/assets/9bc5ac33-8fdd-46e8-8db2-8423f5a86756" />
## After 
<img width="1064" height="428" alt="After" src="https://github.com/user-attachments/assets/c056b1fe-3f8f-4382-a9f6-d9fdaa207a65" />
# Instruction
## Control:
Add bad and trigger process in special lists. When you open trigger process bad is freeze.You also can add process in both lists.

To start work programm press F8 or button *'Start'*

To stop work programm press F10 or button *'Stop'*

# If something wrong
## Make sure:
1. In folder with Lazarus.exe must be lies *BlackList.txt* and *Trigger.txt* <img width="631" height="97" alt="InFolder" src="https://github.com/user-attachments/assets/e80a68a1-a8be-4371-89df-6f3d90056d25" />
2. *BlackList.txt* and *Trigger.txt* save in encoding *ANSI* or *UTF - 16 LE* prefer *ANSI*
3. You should open Lazarus.exe in the name of the administrator
## Contacts(Eng/Rus)
[![Telegram](https://img.shields.io/badge/Telegram-26A5E4?style=for-the-badge&logo=telegram&logoColor=white)](https://t.me/cpp_programsL?direct)


Mail:: **[nkuvshinnikov10@gmail.com](mailto:nkuvshinnikov10@gmail.com)**
