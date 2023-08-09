#include "MIDIClock.c"

unsigned long long sentEvents = 0;
unsigned long totalFrames = 0;
double startTime1 = 0;
double startTime2 = 0;

int PPushback = -1;
unsigned long long* currOffset;
unsigned long int* currEvent;
unsigned long int* trackReadOffset;
BOOL* prepareStep;
BOOL* tempoEvent;
byte* prevEvent;
unsigned long long* trackPosition;

void StartTimeCheck()
{
    double tempT = getTimeMsec();
    if((long)(tempT-startTime1)>=16){
        if((long)(tempT-startTime2)>=1000){
            printf("\nFPS: %.10g",(float)1/((float)(tempT-startTime2)/(float)1000/(float)totalFrames));
            totalFrames = 0;
            startTime2 = tempT;
        }
        startTime1 = tempT;
        char temp[256] = "";
        char num[20];
        char fpstemp[32];
        strcat(temp,prgTitle);
        strcat(temp," | Played events: ");
        sprintf(num, "%llu", sentEvents);
        strcat(temp,num);
        strcat(temp," | BPM: ");
        sprintf(fpstemp, "%.10g", bpm);
        strcat(temp,fpstemp);
        SetConsoleTitle(temp);
    }
}

unsigned long int *cEv;
unsigned long int *tRO;
unsigned long long *cOff;
const unsigned char* track;
BOOL *pStep;
BOOL *tEv;
byte *prevE;
BOOL prepareEvent() {
    if (*pStep == FALSE) return TRUE;
    *pStep = FALSE;
    *cOff = 0;
    *tEv = FALSE;
    unsigned long int event = 0;
    while(TRUE){
        unsigned long int val = 0;
        for (int i = 0; i < 4; i++) {
            byte temp = track[(*tRO)++];
            if (temp > 0x7F) {
                val = (val << 7) | (temp & 0x7F);
            } else {
                val = val << 7 | temp;
                break;
            }
        }
        *cOff += val;
        byte readEvent = track[(*tRO)++];
        if (readEvent < 0x80) {
            PPushback = readEvent;
            readEvent = *prevE;
        }
        byte temp = PPushback;
        *prevE = readEvent;
        BOOL proceed = TRUE;
        byte trackEvent = readEvent & 0b11110000;
        if (readEvent >= 128 && readEvent <= 239) {
            if (trackEvent == 0x90 || trackEvent == 0x80 || trackEvent == 0xA0 || trackEvent == 0xE0 || trackEvent == 0xB0) {
                unsigned char PRead1;
                if (PPushback != -1) {
                    PRead1 = PPushback;
                    PPushback = -1;
                } else {
                    PRead1 = track[(*tRO)++];
                }
                byte tempByte = track[(*tRO)++];
                event = (readEvent | (PRead1 << 8) | (tempByte << 16));
            } else if (trackEvent == 0xC0 || trackEvent == 0xD0) {
                unsigned char PRead1;
                if (PPushback != -1) {
                    PRead1 = PPushback;
                    PPushback = -1;
                } else {
                    PRead1 = track[(*tRO)++];
                }
                event = (readEvent | (PRead1 << 8));
            } else {
                return FALSE;
            }
        } else if (readEvent == 0) {
            return FALSE;
        } else {
            switch (readEvent) {
                case 0b11110000: {
                    unsigned char PRead1;
                    if (PPushback != -1) {
                        PRead1 = PPushback;
                        PPushback = -1;
                    } else {
                        PRead1 = track[(*tRO)++];
                    }
                    while (PRead1 != 0b11110111) {
                        PRead1 = track[(*tRO)++];
                    }
                    break;
                }
                case 0b11110010:
                    *tRO += 2;
                    break;
                case 0b11110011:
                    *tRO += 1;
                    break;
                case 0xFF: {
                    unsigned char PRead1;
                    if (PPushback != -1) {
                        PRead1 = PPushback;
                        PPushback = -1;
                    } else {
                        PRead1 = track[(*tRO)++];
                    }
                    readEvent = PRead1;
                    if (readEvent == 81) {
                        *tRO += 1;
                        for (int i = 0; i != 3; i++) {
                            byte temp = track[(*tRO)++];
                            event = (event << 8) | temp;
                        }
                        *tEv = TRUE;
                    } else if (readEvent == 0x2F) {
                        return FALSE;
                    } else {
                        *tRO += track[(*tRO)++];
                    }
                    break;
                }
            }
        }
        if(event){
            *cEv = event;
            return TRUE;
        }
    }
}
void StartPlayback(){
    double clock = 0;
    BOOL trackFinished[realTracks];
    unsigned int aliveTracks = realTracks;
    currOffset = (unsigned long long*)calloc(realTracks, sizeof(unsigned long long));
    currEvent = (unsigned long int*)calloc(realTracks, sizeof(unsigned long int));
    trackReadOffset = (unsigned long int*)calloc(realTracks, sizeof(unsigned long int));
    trackPosition = (unsigned long long*)calloc(realTracks, sizeof(unsigned long long));
    prevEvent = (byte*)calloc(realTracks, sizeof(byte));
    prepareStep = (BOOL*)calloc(realTracks, sizeof(BOOL));
    tempoEvent = (BOOL*)calloc(realTracks, sizeof(BOOL));
    memset(trackFinished, FALSE, sizeof(trackFinished));
    startTime1 = getTimeMsec();
    startTime2 = getTimeMsec();
    cppq = ppq;
    Clock_Start();
    int (*SendDirectData)(DWORD) = SendDirectDataPtr;
    while(TRUE){
        StartTimeCheck();
        double newClock = Clock_GetTick();
        if(newClock!=clock){
            totalFrames++;
            clock=newClock;
            unsigned long long *tPos = &trackPosition[0];
            cOff = &currOffset[0];
            cEv = &currEvent[0];
            tRO = &trackReadOffset[0];
            pStep = &prepareStep[0];
            tEv = &tempoEvent[0];
            prevE = &prevEvent[0];
            unsigned long long clockUInt64 = (unsigned long long)clock;
            BOOL *tF1 = &trackFinished[0];
            for(unsigned int i = 0; i < realTracks; i++){
                if(*tF1==FALSE){
                    track=tracks[i];
                    while(TRUE){
                        if(prepareEvent()){
                            if(*tPos+*cOff<=clockUInt64){
                                *pStep=TRUE;
                                *tPos+=*cOff;
                                switch(*tEv){
                                    case FALSE:
                                        SendDirectData(*cEv);
                                        sentEvents++;
                                        break;
                                    case TRUE:
                                        Clock_SubmitBPM(*tPos,*cEv);
                                        break;
                                }
                            } else {
                                break;
                            }
                        } else {
                            aliveTracks--;
                            *tF1 = TRUE;
                            break;
                        }
                    }
                }
                *tF1++;*tPos++;*cOff++;*pStep++;*cEv++;*tEv++;*tRO++;*prevE++;
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
