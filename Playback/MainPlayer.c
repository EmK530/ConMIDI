#include "MIDIClock.c"

void StartPlayback(){
    double clock = 0;
    unsigned long totalFrames = 0;
    BOOL trackFinished[realTracks];
    BOOL trackFinished2[realTracks];
    unsigned long long trackOffset[realTracks];
    unsigned long trackIDX[realTracks];
    unsigned long tempoIDX[realTracks];
    memset(trackOffset,0,sizeof(trackOffset));
    memset(trackIDX,0,sizeof(trackIDX));
    memset(tempoIDX,0,sizeof(tempoIDX));
    unsigned int aliveTracks = realTracks;
    for(int i = 0; i < realTracks; i++){
        if(eventCounts[i]==0){
            trackFinished[i] = TRUE;
            aliveTracks--;
        } else {
            trackFinished[i] = FALSE;
        }
        trackFinished2[i] = tempoCounts[i]==0;
    }
    double startTime = getTimeMsec();
    double startTime2 = getTimeMsec();
    cppq = ppq;
    Clock_Start();
    for(int i = 0; i < realTracks; i++){
        trackIDX[i] = 0;
        if(eventCounts[i]==0){
            trackFinished[i] = TRUE;
        } else {
            trackFinished[i] = FALSE;
        }
    }
    int (*SendDirectData)(DWORD) = SendDirectDataPtr;
    while(TRUE){
        long long tempT = getTimeMsec();
        if((long)(tempT-startTime)>=16){
            if((long)(tempT-startTime2)>=1000){
                printf("\nFPS: %.10g",(float)1/((float)(tempT-startTime2)/(float)1000/(float)totalFrames));
                totalFrames = 0;
                startTime2 = tempT;
            }
            startTime = tempT;
            unsigned long long events = 0;
            unsigned long int *tIDX = &trackIDX[0];
            for(int i = 0; i < realTracks; i++){
                events+=*tIDX;
                *tIDX++;
            }
            char temp[256] = "";
            char num[20];
            char fpstemp[32];
            strcat(temp,prgTitle);
            strcat(temp," | Played events: ");
            sprintf(num, "%llu", events);
            strcat(temp,num);
            strcat(temp," | BPM: ");
            sprintf(fpstemp, "%.10g", bpm);
            strcat(temp,fpstemp);
            SetConsoleTitle(temp);
        }
        double newClock = Clock_GetTick();
        if(newClock!=clock){
            totalFrames++;
            clock=newClock;
            unsigned long long clockUInt64 = (unsigned long long)clock;
            unsigned long int *teIDX = &tempoIDX[0];
            unsigned long long *tO = &trackOffset[0];
            unsigned long int *tIDX = &trackIDX[0];
            unsigned long int *eC = &eventCounts[0];
            unsigned long int *tC = &tempoCounts[0];
            BOOL *tF1 = &trackFinished[0];
            BOOL *tF2 = &trackFinished2[0];
            for(int i = 0; i < realTracks; i++){
                if(*tF2==FALSE){
                    unsigned long int temp_teIDX = *teIDX;
                    unsigned long int temp_tC = *tC;
                    struct Tempo *curr2 = Tempos[i] + temp_teIDX;
                    while(curr2->pos <= clock){
                        Clock_SubmitBPM(curr2->pos,curr2->event);
                        temp_teIDX++;
                        if(temp_teIDX>=temp_tC){
                            //printf("\nKilled track %lu",i+1);
                            *tF2 = TRUE;
                            break;
                        } else {
                            curr2++;
                        }
                    }
                    *teIDX=temp_teIDX;
                    *tC=temp_tC;
                }
                if(*tF1==FALSE){
                    unsigned long int temp_tIDX = *tIDX;
                    unsigned long int temp_eC = *eC;
                    unsigned long long temp_tO = *tO;
                    struct SynthEvent *curr = SynthEvents[i] + temp_tIDX;
                    while(temp_tO+curr->pos <= clockUInt64){        
                        SendDirectData(curr->event);
                        temp_tO+=curr->pos;
                        temp_tIDX++;
                        if(temp_tIDX>=temp_eC){
                            //printf("\nKilled track %lu",i+1);
                            aliveTracks--;
                            *tF1 = TRUE;
                            break;
                        } else {
                            curr++;
                        }
                    }
                    *tIDX=temp_tIDX;
                    *eC=temp_eC;
                    *tO=temp_tO;
                }
                *tIDX++;*teIDX++;*tF1++;*tF2++;*eC++;*tC++;*tO++;
            }
        } else {
            usleep(1000);
        }
        if(aliveTracks == 0){
            printf("\nRan out of events, playback finished.");
            sleep(3);
            exit(0);
        }
    }
}
