#include <stdio.h>
#include <windows.h>
#include <unistd.h>
#include "Essentials.c"
#include "Sound/Sound.c"
#include "MIDI/LoadMIDI.c"

FILE *file_ptr;
char version[] = "v2.0.0";
char* title;
int main(){
    printf("ConMIDI %s\n\n",version);
    setupntdll();
    title = concat("ConMIDI ", version);
    prgTitle = title;
    SetConsoleTitle(prgTitle);
    //Check for sound engines
    Sound_Setup();
    //Start sound selection if more than one sound engine is available otherwise auto load only available engine
    if(usables > 1){
        while(TRUE){
            int id = intInput("\nPick sound device ID: ");
            int test = Sound_Init(id);
            if(test==1){
                break;
            }
        }
    } else {
        int i;
        for(i = 0; i < 2; i++){
            if(usable[i]){
                Sound_Init(i+1);
                break;
            }
        }
    }
    //File path input
    char path[260];
    char fixedPath[260];
    while(TRUE){
        printf("\nSelect a MIDI\n");
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = "MIDI Files\0*.mid\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = sizeof(path);
        ofn.lpstrTitle = "Select a MIDI file";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if(!GetOpenFileName(&ofn)){
            printf("No file selected.");
            memset(path, 0, sizeof(path));
            printf("\nEnter file path: ");
            scanf("%260[^\n]",path);
            fflush(stdin);
        }
        removeSymbol(path,'\"',fixedPath);
        if(access(fixedPath, F_OK) != -1){
            break;
        } else {
            printf("Invalid path");
            memset(path, 0, sizeof(path));
        }
    }
    //Consider customizable buffer size
    unsigned int bufSize = 64;
    printf("\nLoading MIDI...");
    LoadMIDI(fixedPath,bufSize);
    return 0;
}
