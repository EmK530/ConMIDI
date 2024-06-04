#include <stdio.h>
#include <windows.h>
#include <unistd.h>
#include "Essentials.h"
#include "Sound\Sound.h"
#include "MIDI\LoadMIDI.h"
#include "Playback\MIDIClock.h"
#include "Playback\MainPlayer.h"

FILE *file_ptr;
char version[] = "v2.0.9";
char *title;
int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        int mode = 0;
        for (int i = 1; i < argc; i++)
        {
            switch (mode)
            {
            case 0:
            {
                char *read = argv[i];
                if (strcmp(read, "-cp") == 0)
                {
                    mode = 1;
                }
                else if (strcmp(read, "-hm") == 0 || strcmp(read, "-hidemeta") == 0)
                {
                    for (int a = 0; a < 10; a++)
                    {
                        metaAllow[a] = FALSE;
                    }
                }
                else if (strcmp(read, "-sm") == 0 || strcmp(read, "-showmeta") == 0)
                {
                    for (int a = 0; a < 10; a++)
                    {
                        metaAllow[a] = TRUE;
                    }
                }
                else if (strcmp(read, "-em") == 0)
                {
                    mode = 2;
                }
                else if (strcmp(read, "-dm") == 0)
                {
                    mode = 3;
                }
                else if (strcmp(read, "-fps") == 0)
                {
                    showFpsOutsideLag = TRUE;
                }
                break;
            }
            case 1:
            {
                unsigned int b;
                sscanf(argv[i], "%u", &b);
                SetConsoleOutputCP(b);
                mode = 0;
                break;
            }
            case 2:
            {
                unsigned int b;
                sscanf(argv[i], "%u", &b);
                if (b > 9)
                {
                    printf("\nCommand line error, -em parameter above 9 (set to %u)", b);
                    exit(0);
                }
                metaAllow[b] = TRUE;
                mode = 0;
                break;
            }
            case 3:
            {
                unsigned int b;
                sscanf(argv[i], "%u", &b);
                if (b > 9)
                {
                    printf("\nCommand line error, -dm parameter above 9 (set to %u)", b);
                    exit(0);
                }
                metaAllow[b] = FALSE;
                mode = 0;
                break;
            }
            }
        }
    }
    printf("ConMIDI %s\n\n", version);
    setupntdll();
    title = concat("ConMIDI ", version);
    prgTitle = title;
    SetConsoleTitle(prgTitle);
    // Check for sound engines
    Sound_Setup();
    // Start sound selection if more than one sound engine is available otherwise auto load only available engine
    if (usables > 1)
    {
        while (TRUE)
        {
            int id = intInput("\nPick sound device ID: ");
            int test = Sound_Init(id);
            if (test == 1)
            {
                break;
            }
        }
    }
    else
    {
        int i;
        for (i = 0; i < 2; i++)
        {
            if (usable[i])
            {
                Sound_Init(i + 1);
                break;
            }
        }
    }
    // File path input
    char path[260];
    char fixedPath[260];
    while (TRUE)
    {
        printf("\nSelect a MIDI\n");
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = "MIDI Files\0*.mid\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = sizeof(path);
        ofn.lpstrTitle = "Select a MIDI file";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (!GetOpenFileName(&ofn))
        {
            printf("No file selected.");
            memset(path, 0, sizeof(path));
            printf("\nEnter file path: ");
            scanf("%260[^\n]", path);
            fflush(stdin);
        }
        removeSymbol(path, '\"', fixedPath);
        if (access(fixedPath, F_OK) != -1)
        {
            break;
        }
        else
        {
            printf("Invalid path");
            memset(path, 0, sizeof(path));
        }
    }
    unsigned int bufSize = 64;
    printf("\nLoading MIDI...");
    LoadMIDI(fixedPath, bufSize);
    return 0;
}
