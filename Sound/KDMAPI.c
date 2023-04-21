#include <stdio.h>
#include <windows.h>

HMODULE KDMAPI_libHandle;
typedef int(__stdcall *KDM_INIT)();
typedef int(__stdcall *KDM_SEND)(DWORD);
KDM_INIT KDMAPI_InitializeKDMAPIStream;
KDM_INIT KDMAPI_TerminateKDMAPIStream;
KDM_INIT KDMAPI_ResetKDMAPIStream;
KDM_SEND KDMAPI_SendDirectData;
int KDMAPI_Setup(){
    if ((KDMAPI_libHandle = LoadLibrary("OmniMIDI\\OmniMIDI")) == NULL)
    {
        printf("KDMAPI not available.\n");
        return 0;
    }
    if ((KDMAPI_InitializeKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "InitializeKDMAPIStream")) == NULL)
    {
        printf("GetProcAddress failed for KDMAPI.\n");
        printf("%d\n", GetLastError());
        return 0;
    }
    static KDM_SEND cached_KDMAPI_SendDirectData = NULL;
    if (!cached_KDMAPI_SendDirectData)
    {
        cached_KDMAPI_SendDirectData = (KDM_SEND)GetProcAddress(KDMAPI_libHandle, "SendDirectData");
        if (!cached_KDMAPI_SendDirectData)
        {
            printf("GetProcAddress failed for SendDirectData.\n");
            printf("%d\n", GetLastError());
            return 0;
        }
    }
    KDMAPI_TerminateKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "TerminateKDMAPIStream");
    KDMAPI_ResetKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "ResetKDMAPIStream");
    printf("KDMAPI FUNCTIONAL\n");
    KDMAPI_SendDirectData = cached_KDMAPI_SendDirectData;
    return 1;
}
