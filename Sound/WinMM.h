#ifndef WINMM_H
#define WINMM_H

#include <windows.h>
#include "..\Essentials.h"

int WinMM_Setup();

void WinMM_SendDirectData(unsigned long int event);
int WinMM_SendDirectLongData(MIDIHDR* ptr, unsigned int size);
int WinMM_PrepareLongData(MIDIHDR* ptr, unsigned int size);
int WinMM_UnprepareLongData(MIDIHDR* ptr, unsigned int size);

int WinMM_Init();

#endif