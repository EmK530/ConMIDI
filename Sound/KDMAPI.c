#include <stdio.h>
#include <windows.h>

HMODULE KDMAPI_libHandle;
typedef int(__stdcall *KDM_INIT)();
typedef void(__stdcall *KDM_SEND)(unsigned long int);
typedef int(__stdcall *KDM_LSEND)(MIDIHDR *, unsigned int);
KDM_INIT KDMAPI_InitializeKDMAPIStream;
KDM_INIT KDMAPI_TerminateKDMAPIStream;
KDM_INIT KDMAPI_ResetKDMAPIStream;
KDM_SEND KDMAPI_SendDirectData;
KDM_LSEND KDMAPI_SendDirectLongData;
KDM_LSEND KDMAPI_PrepareLongData;
KDM_LSEND KDMAPI_UnprepareLongData;
int KDMAPI_Setup()
{
    if ((KDMAPI_libHandle = LoadLibrary("OmniMIDI")) == NULL)
    {
        printf("KDMAPI not available.\n");
        return 0;
    }
    if ((KDMAPI_InitializeKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "InitializeKDMAPIStream")) == NULL)
    {
        printf("GetProcAddress failed for KDMAPI InitializeKDMAPIStream.\n");
        return 0;
    }
    if ((KDMAPI_SendDirectData = (KDM_SEND)GetProcAddress(KDMAPI_libHandle, "SendDirectData")) == NULL)
    {
        printf("GetProcAddress failed for KDMAPI SendDirectData.\n");
        return 0;
    }
    if ((KDMAPI_SendDirectLongData = (KDM_LSEND)GetProcAddress(KDMAPI_libHandle, "SendDirectLongData")) == NULL)
    {
        printf("GetProcAddress failed for KDMAPI SendDirectLongData.\n");
        return 0;
    }
    if ((KDMAPI_PrepareLongData = (KDM_LSEND)GetProcAddress(KDMAPI_libHandle, "PrepareLongData")) == NULL)
    {
        printf("GetProcAddress failed for KDMAPI PrepareLongData.\n");
        return 0;
    }
    if ((KDMAPI_UnprepareLongData = (KDM_LSEND)GetProcAddress(KDMAPI_libHandle, "UnprepareLongData")) == NULL)
    {
        printf("GetProcAddress failed for KDMAPI UnprepareLongData.\n");
        return 0;
    }
    if ((KDMAPI_TerminateKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "TerminateKDMAPIStream")) == NULL)
    {
        printf("GetProcAddress failed for KDMAPI TerminateKDMAPIStream.\n");
        return 0;
    }
    if ((KDMAPI_ResetKDMAPIStream = (KDM_INIT)GetProcAddress(KDMAPI_libHandle, "ResetKDMAPIStream")) == NULL)
    {
        printf("GetProcAddress failed for KDMAPI ResetKDMAPIStream.\n");
        return 0;
    }
    printf("KDMAPI FUNCTIONAL\n");
    return 1;
}
