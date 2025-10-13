<head>
<link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css" rel="stylesheet">
</head>

# LPKReact
LPK React Version
## Introduction
This is a branch version of LPK that turn Loop Detect into Inline hook.  
LPK is a project that use to monitor or filter Win32 API calls

## Environment
Test Environment Only  
Windows 10 /Windows 11(Not tested Yet)  
(Actually it might works on earlier version of Windows)  

## Feature
- __OpenProcesMonitor__ : <span style="color: green;"><i class="fas fa-check-circle"> OK</span>
- __DescendantProcessHook__ : <span style="color: green;"><i class="fas fa-check-circle"></i> OK</span>
- __ProcessStatics__ : <span style="color: green;"><i class="fas fa-check-circle"></i> OK</span>
- __ProtectedPIDTable__ :<span style="color: green;"><i class="fas fa-check-circle"></i> OK</span>
- __Injection__ :
- - MessageHook : <span style="color: green;"><i class="fas fa-check-circle"></i> OK</span>
  - CreateRemoteThread : <span style="color: green;"><i class="fas fa-check-circle"></i> OK</span>
  - Registry : <span style="color: orange;"><i class="fas fa-clock"></i> Pending</span>

## Usage
- Rename Detour Test 2.dll into LPK64.dll   
- Put LPK64.dll into %SystemRoot%\System32\  
__Notice__ : You might need to use LoadLPK.h to write a program to adjust the config inside DLL and injects it

## Notice
This project is not releated to any projects that have similar names expcept my LPK

## License
This project use MIT License, details in LICENSE
