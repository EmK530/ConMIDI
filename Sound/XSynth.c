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
    if ((XSynth_libHandle = LoadLibrary("XSynth.dll")) == NULL)
    {
        printf("XSynth not available.\n");
        return 0;
    }
    if ((XSynth_InitializeKDMAPIStream = (XS_INIT)GetProcAddress(XSynth_libHandle, "InitializeKDMAPIStream")) == NULL)
    {
        printf("GetProcAddress failed for XSynth.\n");
        printf("%d\n", GetLastError());
        return 0;
    }
    static XS_SEND cached_XSynth_SendDirectData = NULL;
    if (!cached_XSynth_SendDirectData)
    {
        cached_XSynth_SendDirectData = (XS_SEND)GetProcAddress(XSynth_libHandle, "SendDirectData");
        if (!cached_XSynth_SendDirectData)
        {
            printf("GetProcAddress failed for SendDirectData.\n");
            printf("%d\n", GetLastError());
            return 0;
        }
    }
    XSynth_SendDirectData = cached_XSynth_SendDirectData;
    XSynth_TerminateKDMAPIStream = (XS_INIT)GetProcAddress(XSynth_libHandle, "TerminateKDMAPIStream");
    XSynth_ResetKDMAPIStream = (XS_INIT)GetProcAddress(XSynth_libHandle, "ResetKDMAPIStream");
    printf("XSynth FUNCTIONAL\n");
    return 1;
}