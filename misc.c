#include "stdio.h"
#include "defs.h"
#include "windows.h"

int GetTimeMs() {   // Time the process
    return GetTickCount();
}