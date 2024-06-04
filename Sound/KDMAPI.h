#ifndef KDMAPI_H
#define KDMAPI_H

#include <windows.h>

typedef int(__stdcall *KDM_INIT)();
typedef void(__stdcall *KDM_SEND)(unsigned long int);
typedef int(__stdcall *KDM_LSEND)(MIDIHDR*, unsigned int);
extern KDM_INIT KDMAPI_InitializeKDMAPIStream;
extern KDM_SEND KDMAPI_SendDirectData;
extern KDM_LSEND KDMAPI_SendDirectLongData;
extern KDM_LSEND KDMAPI_PrepareLongData;
extern KDM_LSEND KDMAPI_UnprepareLongData;
int KDMAPI_Setup();

#endif