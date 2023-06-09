#include <stdio.h>
#include <windows.h>
#include <unistd.h>
#include "Essentials.c"
#include "Sound/Sound.c"
#include "MIDI/LoadMIDI.c"

FILE *file_ptr;
char version[] = "Prerelease 2";
char* title;
int main(){
    printf("ConMIDI %s\n\n",version);
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
    char path[256];
    char fixedPath[256];
    while(TRUE){
        printf("\n[Prompt 1 / 2] Enter file path: ");
        scanf("%256[^\n]",path);
        fflush(stdin);
        removeSymbol(path,'\"',fixedPath);
        if(access(fixedPath, F_OK) != -1){
            break;
        } else {
            printf("Invalid path\n");
        }
    }
    //Threshold input
    int threshold;
    while(TRUE){
        threshold = intInput("\n[Prompt 2 / 2] Enter note threshold (0-127): ");
        //threshold = 0;
        if(threshold>=0 && threshold<=127){
            break;
        } else {
            printf("Invalid input\n");
        }
    }
    //Consider customizable buffer size
    unsigned int bufSize = 64;
    printf("\nLoading MIDI...");
    LoadMIDI(fixedPath,threshold,bufSize);
    return 0;
}