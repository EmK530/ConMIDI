#include "MIDIClock.c"

void StartPlayback(){
    double clock = 0;
    unsigned long totalFrames = 0;
    BOOL trackFinished[realTracks];
    BOOL trackFinished2[realTracks];
    unsigned long trackIDX[realTracks];
    unsigned long tempoIDX[realTracks];
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
    long long startTime = timeInMilliseconds();
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
    while(TRUE){
        long long tempT = timeInMilliseconds();
        if((long)(tempT-startTime)>=1000){
            printf("\nFPS: %f",(float)1/((float)(tempT-startTime)/(float)1000/(float)totalFrames));
            startTime = tempT;
            totalFrames = 0;
        }
        double newClock = Clock_GetTick();
        totalFrames++;
        if(newClock!=clock){
            clock=newClock;
            unsigned long int *teIDX = &tempoIDX[0];
            unsigned long int *tIDX = &trackIDX[0];
            unsigned long int *eC = &eventCounts[0];
            unsigned long int *tC = &tempoCounts[0];
            BOOL *tF1 = &trackFinished[0];
            BOOL *tF2 = &trackFinished2[0];
            for(int i = 0; i < realTracks; i++){
                if(*tF2==FALSE){
                    struct SynthEvent *curr2 = Tempos[i] + *teIDX;
                    while(curr2->pos <= clock){
                        Clock_SubmitBPM(curr2->pos,curr2->event);
                        *teIDX+=1;
                        if(*teIDX>=*tC){
                            //printf("\nKilled track %lu",i+1);
                            *tF2 = TRUE;
                            break;
                        } else {
                            curr2++;
                        }
                    }
                }
                if(*tF1==FALSE){
                    struct SynthEvent *curr = SynthEvents[i] + *tIDX;
                    while(curr->pos <= clock){        
                        (*SendDirectDataPtr)(curr->event);
                        *tIDX+=1;
                        if(*tIDX>=*eC){
                            //printf("\nKilled track %lu",i+1);
                            aliveTracks--;
                            *tF1 = TRUE;
                            break;
                        } else {
                            curr++;
                        }
                    }
                }
                *tIDX++;*teIDX++;*tF1++;*tF2++;*eC++;*tC++;
            }
        }
        if(aliveTracks == 0){
            printf("\nRan out of events, ending playback in 3s.");
            sleep(3);
            exit(0);
        }
    }
}