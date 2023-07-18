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
    if ((KDMAPI_libHandle = LoadLibrary("OmniMIDI")) == NULL){printf("KDMAPI not available.\n");return 0;}
    if ((KDMAPI_InitializeKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "InitializeKDMAPIStream")) == NULL){printf("GetProcAddress failed for KDMAPI InitializeKDMAPIStream.\n");return 0;}
    static KDM_SEND cached_KDMAPI_SendDirectData = NULL;
    if (!cached_KDMAPI_SendDirectData){
        cached_KDMAPI_SendDirectData = (KDM_SEND)GetProcAddress(KDMAPI_libHandle, "SendDirectData");
        if (!cached_KDMAPI_SendDirectData){
            printf("GetProcAddress failed for KDMAPI SendDirectData.\n");
            return 0;
        }
    }
    if ((KDMAPI_TerminateKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "TerminateKDMAPIStream")) == NULL){printf("GetProcAddress failed for KDMAPI TerminateKDMAPIStream.\n");return 0;}
    if ((KDMAPI_ResetKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "ResetKDMAPIStream")) == NULL){printf("GetProcAddress failed for KDMAPI ResetKDMAPIStream.\n");return 0;}
    printf("KDMAPI FUNCTIONAL\n");
    KDMAPI_SendDirectData = cached_KDMAPI_SendDirectData;
    return 1;
}
