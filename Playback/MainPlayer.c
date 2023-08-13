#include "MIDIClock.c"

unsigned long long sentEvents = 0;
unsigned long totalFrames = 0;
double startTime1 = 0;
double startTime2 = 0;

int PPushback = -1;
unsigned long long* currOffset;
unsigned long int* currEvent;
unsigned long int* trackReadOffset;
unsigned char **trackReads;
BOOL* prepareStep;
BOOL* tempoEvent;
byte* prevEvent;
unsigned long long* trackPosition;

char* AddCommas(char* num){
    int len = strlen(num);
    int commas = (len-1)/3;
    int len2 = len+commas;
    char *newnum = (char *)malloc(len2+1);
    int j,k = 0;
    for(int i=len-1; i>=0; i--){
        newnum[j++] = num[i];
        k++;
        if(k%3==0&&i>0){
            newnum[j++] = ',';
        }
    }
    newnum[len2] = '\0';
    for (int i = 0; i < len2 / 2; i++) {
        char temp = newnum[i];
        newnum[i] = newnum[len2-i-1];
        newnum[len2-i-1] = temp;
    }
    return newnum;
}

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
        char* num2 = AddCommas(num);
        strcat(temp,num2);
        free(num2);
        strcat(temp," | BPM: ");
        sprintf(fpstemp, "%.10g", bpm);
        strcat(temp,fpstemp);
        SetConsoleTitle(temp);
    }
}

unsigned long int *cEv;
unsigned long int *tRO;
unsigned char *tR;
unsigned long long *cOff;
BOOL *pStep;
BOOL *tEv;
byte *prevE;
void StartPlayback(){
    double clock = 0;
    BOOL trackFinished[realTracks];
    unsigned int aliveTracks = realTracks;
    currOffset = (unsigned long long*)calloc(realTracks, sizeof(unsigned long long));
    currEvent = (unsigned long int*)calloc(realTracks, sizeof(unsigned long int));
    trackReadOffset = (unsigned long int*)calloc(realTracks, sizeof(unsigned long int));
    trackReads = (unsigned char **)malloc(realTracks * sizeof(unsigned char *));
    for(int i = 0; i < realTracks; i++){
        trackReads[i] = &tracks[i][0];
    }
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
            pStep = &prepareStep[0];
            tEv = &tempoEvent[0];
            prevE = &prevEvent[0];
            unsigned long long clockUInt64 = (unsigned long long)clock;
            BOOL *tF1 = &trackFinished[0];
            for(unsigned int i = 0; i < realTracks; i++){
                if(*tF1==FALSE){
                    tR = trackReads[i];
                    unsigned long long tempPos = *tPos;
                    BOOL doloop = TRUE;
                    BOOL tempstep = *pStep;
                    while(TRUE){
                        if(tempstep){
                            unsigned long int addOff = 0;
                            unsigned long int event = 0;
                            byte tempPrev = *prevE;
                            while(doloop){
                                unsigned long int val = 0;
                                for (int i = 0; i < 4; i++) {
                                    byte temp = *(tR++);
                                    if (temp > 0x7F) {
                                        val = (val << 7) | (temp & 0x7F);
                                    } else {
                                        val = val << 7 | temp;
                                        break;
                                    }
                                }
                                addOff+=val;
                                byte readEvent = *(tR++);
                                if (readEvent < 0x80) {
                                    PPushback = readEvent;
                                    readEvent = tempPrev;
                                }
                                byte temp = PPushback;
                                tempPrev = readEvent;
                                byte trackEvent = readEvent & 0b11110000;
                                if (readEvent >= 128 && readEvent <= 239) {
                                    if (trackEvent == 0x90 || trackEvent == 0x80 || trackEvent == 0xA0 || trackEvent == 0xE0 || trackEvent == 0xB0) {
                                        unsigned char PRead1;
                                        if (PPushback != -1) {
                                            PRead1 = PPushback;
                                            PPushback = -1;
                                        } else {
                                            PRead1 = *(tR++);
                                        }
                                        byte tempByte = *(tR++);
                                        if(tempPos+addOff<=clockUInt64){
                                            SendDirectData((readEvent | (PRead1 << 8) | (tempByte << 16)));
                                            sentEvents++;
                                            tempPos+=addOff;
                                            addOff=0;
                                        } else {
                                            *cEv = (readEvent | (PRead1 << 8) | (tempByte << 16));
                                            doloop=FALSE;
                                            tempstep=FALSE;
                                            *tEv = FALSE;
                                            break;
                                        }
                                    } else if (trackEvent == 0xC0 || trackEvent == 0xD0) {
                                        unsigned char PRead1;
                                        if (PPushback != -1) {
                                            PRead1 = PPushback;
                                            PPushback = -1;
                                        } else {
                                            PRead1 = *(tR++);
                                        }
                                        if(tempPos+addOff<=clockUInt64){
                                            SendDirectData((readEvent | (PRead1 << 8)));
                                            sentEvents++;
                                            tempPos+=addOff;
                                            addOff=0;
                                        } else {
                                            *cEv = (readEvent | (PRead1 << 8));
                                            doloop=FALSE;
                                            tempstep=FALSE;
                                            *tEv = FALSE;
                                            break;
                                        }
                                    } else {
                                        doloop=FALSE;
                                        break;
                                    }
                                } else if (readEvent == 0) {
                                    doloop=FALSE;
                                    break;
                                } else {
                                    switch (readEvent) {
                                        case 0b11110000: {
                                            unsigned char PRead1;
                                            if (PPushback != -1) {
                                                PRead1 = PPushback;
                                                PPushback = -1;
                                            } else {
                                                PRead1 = *(tR++);
                                            }
                                            while (PRead1 != 0b11110111) {
                                                PRead1 = *(tR++);
                                            }
                                            break;
                                        }
                                        case 0b11110010:
                                            tR += 2;
                                            break;
                                        case 0b11110011:
                                            tR++;
                                            break;
                                        case 0xFF: {
                                            unsigned char PRead1;
                                            if (PPushback != -1) {
                                                PRead1 = PPushback;
                                                PPushback = -1;
                                            } else {
                                                PRead1 = *(tR++);
                                            }
                                            readEvent = PRead1;
                                            if (readEvent == 81) {
                                                tR++;
                                                event = 0;
                                                for (int i = 0; i != 3; i++) {
                                                    byte temp = *(tR++);
                                                    event = (event << 8) | temp;
                                                }
                                                if(tempPos+addOff<=clockUInt64){
                                                    tempPos+=addOff;
                                                    Clock_SubmitBPM(tempPos,event);
                                                    addOff=0;
                                                } else {
                                                    *cEv = event;
                                                    doloop=FALSE;
                                                    tempstep=FALSE;
                                                    *tEv = TRUE;
                                                    break;
                                                }
                                            } else if (readEvent == 0x2F) {
                                                doloop=FALSE;
                                                *tF1=TRUE;
                                                break;
                                            } else {
                                                tR += *(tR++);
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            *cOff=addOff;
                            *tPos=tempPos;
                            *prevE=tempPrev;
                            if(!doloop){
                                break;
                            }
                        } else {
                            unsigned long int tempOff = *cOff;
                            if(tempPos+tempOff<=clockUInt64){
                                tempstep=TRUE;
                                tempPos+=tempOff;
                                if(*tEv==FALSE){
                                    SendDirectData(*cEv);
                                    sentEvents++;
                                } else {
                                    Clock_SubmitBPM(tempPos,*cEv);
                                }
                            } else {
                                break;
                            }
                        }
                    }
                    *pStep=tempstep;
                    *tPos=tempPos;
                    trackReads[i] = tR;
                }
                *tF1++;*tPos++;*cOff++;*pStep++;*cEv++;*prevE++;*tEv++;
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
