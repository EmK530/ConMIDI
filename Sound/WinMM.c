#include <stdio.h>
#include <windows.h>
#include "..\Essentials.h"

HMODULE WinMM_libHandle;
typedef int(__stdcall *WM_1)();
typedef int(__stdcall *WM_2)(UINT_PTR, LPMIDIOUTCAPSW, UINT);
typedef int(__stdcall *WM_3)(LPHMIDIOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD);
typedef void(__stdcall *WM_4)(HMIDIOUT, unsigned long int);
typedef int(__stdcall *WM_5)(HMIDIOUT, MIDIHDR *, UINT);
WM_1 WinMM_midiOutGetNumDevs;
WM_2 WinMM_midiOutGetDevCaps;
WM_3 WinMM_midiOutOpen;
WM_4 WinMM_midiOutShortMsg;
WM_5 WinMM_midiOutLongMsg;
WM_5 WinMM_midiOutPrepareHeader;
WM_5 WinMM_midiOutUnprepareHeader;
HMIDIOUT handle;

int WinMM_Setup()
{
    if ((WinMM_libHandle = LoadLibrary("winmm.dll")) == NULL)
    {
        printf("WinMM not available.\n");
        return 0;
    }
    if ((WinMM_midiOutGetNumDevs = (WM_1)GetProcAddress(WinMM_libHandle, "midiOutGetNumDevs")) == NULL)
    {
        printf("GetProcAddress failed for WinMM midiOutGetNumDevs.\n");
        return 0;
    }
    if ((WinMM_midiOutGetDevCaps = (WM_2)GetProcAddress(WinMM_libHandle, "midiOutGetDevCapsW")) == NULL)
    {
        printf("GetProcAddress failed for WinMM midiOutGetDevCapsW.\n");
        return 0;
    }
    if ((WinMM_midiOutOpen = (WM_3)GetProcAddress(WinMM_libHandle, "midiOutOpen")) == NULL)
    {
        printf("GetProcAddress failed for WinMM midiOutOpen.\n");
        return 0;
    }
    if ((WinMM_midiOutShortMsg = (WM_4)GetProcAddress(WinMM_libHandle, "midiOutShortMsg")) == NULL)
    {
        printf("GetProcAddress failed for WinMM midiOutShortMsg.\n");
        return 0;
    }
    if ((WinMM_midiOutLongMsg = (WM_5)GetProcAddress(WinMM_libHandle, "midiOutLongMsg")) == NULL)
    {
        printf("GetProcAddress failed for WinMM midiOutLongMsg.\n");
        return 0;
    }
    if ((WinMM_midiOutPrepareHeader = (WM_5)GetProcAddress(WinMM_libHandle, "midiOutPrepareHeader")) == NULL)
    {
        printf("GetProcAddress failed for WinMM midiOutPrepareHeader.\n");
        return 0;
    }
    if ((WinMM_midiOutUnprepareHeader = (WM_5)GetProcAddress(WinMM_libHandle, "midiOutUnprepareHeader")) == NULL)
    {
        printf("GetProcAddress failed for WinMM midiOutUnprepareHeader.\n");
        return 0;
    }
    printf("WinMM FUNCTIONAL\n");
    return 1;
}

void WinMM_SendDirectData(unsigned long int event)
{
    WinMM_midiOutShortMsg(handle, event);
}
int WinMM_SendDirectLongData(MIDIHDR *ptr, unsigned int size)
{
    WinMM_midiOutLongMsg(handle, ptr, size);
}
int WinMM_PrepareLongData(MIDIHDR *ptr, unsigned int size)
{
    WinMM_midiOutPrepareHeader(handle, ptr, size);
}
int WinMM_UnprepareLongData(MIDIHDR *ptr, unsigned int size)
{
    WinMM_midiOutUnprepareHeader(handle, ptr, size);
}

int WinMM_Init()
{
    int Devices = WinMM_midiOutGetNumDevs();
    if (Devices == 0)
    {
        printf("\nNo WinMM devices found!\n");
        return 0;
    }
    printf("\nAvailable devices:\n");
    for (int i = 0; i < Devices; i++)
    {
        MIDIOUTCAPSW midiOutCaps;
        if (WinMM_midiOutGetDevCaps(i, &midiOutCaps, sizeof(midiOutCaps)) == MMSYSERR_NOERROR)
        {
            wchar_t *pszPname = midiOutCaps.szPname;
            wprintf(L"%i: %ls\n", i + 1, pszPname);
        }
        else
        {
            wprintf(L"\nError getting MIDI output device capabilities.");
        }
    }
    int id = intInput("\nPick WinMM device ID: ");
    if (WinMM_midiOutOpen(&handle, id - 1, 0, 0, 0) == MMSYSERR_NOERROR)
    {
        return 1;
    }
    else
    {
        printf("Could not open MIDI output device.\n");
        return 0;
    }
}
