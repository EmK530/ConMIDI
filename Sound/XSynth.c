#include <stdio.h>
#include <windows.h>

HMODULE XSynth_libHandle;
typedef int(__stdcall *XS_INIT)();
typedef int(__stdcall *XS_SEND)(DWORD);
XS_INIT XSynth_InitializeKDMAPIStream;
XS_INIT XSynth_TerminateKDMAPIStream;
XS_INIT XSynth_ResetKDMAPIStream;
XS_SEND XSynth_SendDirectData;
int XSynth_Setup(){
    if ((XSynth_libHandle = LoadLibrary("XSynth.dll")) == NULL){printf("XSynth not available.\n");return 0;}
    if ((XSynth_InitializeKDMAPIStream = (XS_INIT)GetProcAddress(XSynth_libHandle, "InitializeKDMAPIStream")) == NULL){printf("GetProcAddress failed for XSynth InitializeKDMAPIStream.\n");return 0;}
    if ((XSynth_SendDirectData = (XS_INIT)GetProcAddress(XSynth_libHandle, "SendDirectData")) == NULL){printf("GetProcAddress failed for XSynth SendDirectData.\n");return 0;}
    if ((XSynth_TerminateKDMAPIStream = (XS_INIT)GetProcAddress(XSynth_libHandle, "TerminateKDMAPIStream")) == NULL){printf("GetProcAddress failed for XSynth TerminateKDMAPIStream.\n");return 0;}
    if ((XSynth_ResetKDMAPIStream = (XS_INIT)GetProcAddress(XSynth_libHandle, "ResetKDMAPIStream")) == NULL){printf("GetProcAddress failed for XSynth ResetKDMAPIStream.\n");return 0;}
    printf("XSynth FUNCTIONAL\n");
    return 1;
}
