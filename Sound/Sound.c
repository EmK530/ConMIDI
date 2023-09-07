#include <stdio.h>
#include <windows.h>
#include "KDMAPI.c"
#include "WinMM.c"

void (*SendDirectDataPtr)(unsigned long int a);
int (*SendDirectLongDataPtr)(MIDIHDR* a, unsigned int b);
int (*PrepareLongDataPtr)(MIDIHDR* a, unsigned int b);
int (*UnprepareLongDataPtr)(MIDIHDR* a, unsigned int b);

int test = 0;
BOOL usable[2];
int usables = 0;
void Sound_Setup(){
    if(KDMAPI_Setup()==1){usable[0]=TRUE;usables++;};
    if(WinMM_Setup()==1){usable[1]=TRUE;usables++;};
    if(usables > 1){
        printf("\nSound selection:\n");
        if(usable[0]){printf("1: KDMAPI\n");}
        if(usable[1]){printf("2: WinMM\n");}
    }
    /*
    if(usable[0]){
        int res = KDMAPI_InitializeKDMAPIStream();
        if(res==1){
            printf("KDMAPI Initialized\n");
        }
        test=1;
    } else {
        printf("Did not initialize KDMAPI due to import failure.\n");
    }
    */
}
int Sound_Init(int id){
    if(usable[id-1]){
        switch(id){
            case 1:
                {
                    int res = KDMAPI_InitializeKDMAPIStream();
                    if(res==1){
                        printf("KDMAPI Initialized\n");
                        SendDirectDataPtr = KDMAPI_SendDirectData;
                        SendDirectLongDataPtr = KDMAPI_SendDirectLongData;
                        PrepareLongDataPtr = KDMAPI_PrepareLongData;
                        UnprepareLongDataPtr = KDMAPI_UnprepareLongData;
                        test=id;
                        return 1;
                    } else {
                        printf("KDMAPI failed to initialize\n");
                        return 0;
                    }
                }
                break;
            case 2:
                {
                    int res = WinMM_Init();
                    if(res==1){
                        printf("WinMM Initialized\n");
                        SendDirectDataPtr = WinMM_SendDirectData;
                        SendDirectLongDataPtr = WinMM_SendDirectLongData;
                        PrepareLongDataPtr = WinMM_PrepareLongData;
                        UnprepareLongDataPtr = WinMM_UnprepareLongData;
                        test=id;
                        return 1;
                    } else {
                        printf("WinMM failed to initialize\n");
                        return 0;
                    }
                }
            default:
                printf("Unknown device ID\n");
                return 0;
                break;
        }
    } else {
        printf("Selected device ID is not available\n");
        return 0;
    }
}
