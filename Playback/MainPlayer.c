#include "MIDIClock.c"

unsigned long long sentEvents = 0;
unsigned long totalFrames = 0;
double startTime1 = 0;
double startTime2 = 0;
BOOL metaAllow[10] = {FALSE,FALSE,FALSE,FALSE,FALSE,TRUE,FALSE,FALSE,FALSE,FALSE};
BOOL showFpsOutsideLag = TRUE;

unsigned long int* currEvent;
unsigned long int* trackReadOffset;
unsigned char **trackReads;
BOOL* prepareStep;
unsigned char* eventType;
byte* prevEvent;
unsigned long long* trackPosition;

char* AddCommas(const char* num) {
    int len = strlen(num);
    int commas = (len - 1) / 3;
    int len2 = len + commas;
    char* newnum = (char*)malloc(len2 + 1);
    newnum[len2] = '\0';
    int i = len - 1;
    int j = len2 - 1;
    int k = 0;
    while (i >= 0) {
        newnum[j] = num[i];
        i--;
        j--;
        k++;
        if (k == 3 && i >= 0) {
            newnum[j] = ',';
            j--;
            k = 0;
        }
    }
    return newnum;
}

void StartTimeCheck()
{
    double tempT = getTimeMsec();
    if((long)(tempT-startTime1)>=16){
        if((long)(tempT-startTime2)>=1000){
            float calc = (float)1/((float)(tempT-startTime2)/(float)1000/(float)totalFrames);
            if(showFpsOutsideLag){
                printf("\nFPS: %.10g",calc);
            } else {
                if(calc<60){
                    printf("\nLag detected, FPS: %.10g",calc);
                }
            }
            //printf("\nFPS: %.10g",);
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

void metaPrint(byte meta){
    switch(meta){
        case 0x01:
            printf("\nText: ");
            break;
        case 0x02:
            printf("\nCopyright: ");
            break;
        case 0x05:
            printf("\nLyric: ");
            break;
        case 0x06:
            printf("\nMarker: ");
            break;
        case 0x07:
            printf("\nCue: ");
            break;
        case 0x08:
            printf("\nProgram: ");
            break;
        default:
            printf("\nUnknown: ");
            break;
    }
}
void (*SendDirectData)(unsigned long int);
int (*SendDirectLongData)(MIDIHDR* a, unsigned int b);
int (*PrepareLongData)(MIDIHDR* a, unsigned int b);
int (*UnprepareLongData)(MIDIHDR* a, unsigned int b);
unsigned char *tR;
void handleSysEx(){
    unsigned int size = 64;
    unsigned char* arr=(unsigned char*)malloc(size);
    int pos = 1;
    arr[0] = 0xF0;
    tR++;
    while((arr[pos]=*(tR++))!=0b11110111){
        pos++;
        if(pos>=size){
            size *= 2;
            arr = (unsigned char*)realloc(arr, size);
        }
    }
    MIDIHDR longdata;
    memset(&longdata, 0, sizeof(longdata));
    longdata.lpData = (LPBYTE)&arr[0];
    longdata.dwBufferLength = pos+1;
    longdata.dwBytesRecorded = pos+1;
    longdata.dwFlags = 0;
    UINT error = PrepareLongData(&longdata,sizeof(longdata));
    if(!error){
        //printf("\nPrepared SysEx");
        error = SendDirectLongData(&longdata,sizeof(longdata));
        //printf("\n");
        //for(int a = 0; a < pos+1; a++){
            //printf("%X ",arr[a]);
        //}
        if(error){
            printf("\nCould not play SysEx, error %lu", error);
        } else {
            sentEvents++;
        }
        while(MIDIERR_STILLPLAYING == UnprepareLongData(&longdata,sizeof(longdata))){
            printf("\nFailed to unprepare SysEx, retrying...");
            usleep(1000);
        }
    } else {
        printf("\nFailed to prepare SysEx");
    }
    free(arr);
}
unsigned long int *cEv;
unsigned long int *tRO;
BOOL *pStep;
unsigned char *eT;
byte *prevE;
void StartPlayback(){
    double clock = 0;
    BOOL trackFinished[realTracks];
    unsigned int aliveTracks = realTracks;
    currEvent = (unsigned long int*)calloc(realTracks, sizeof(unsigned long int));
    trackReadOffset = (unsigned long int*)calloc(realTracks, sizeof(unsigned long int));
    trackReads = (unsigned char **)malloc(realTracks * sizeof(unsigned char *));
    for(int i = 0; i < realTracks; i++){
        trackReads[i] = &tracks[i][0];
    }
    trackPosition = (unsigned long long*)calloc(realTracks, sizeof(unsigned long long));
    prevEvent = (byte*)calloc(realTracks, sizeof(byte));
    prepareStep = (BOOL*)calloc(realTracks, sizeof(BOOL));
    eventType = (unsigned char*)calloc(realTracks, sizeof(unsigned char));
    memset(trackFinished,FALSE,sizeof(BOOL)*realTracks);
    memset(prepareStep,TRUE,sizeof(BOOL)*realTracks);
    startTime1 = getTimeMsec();
    startTime2 = getTimeMsec();
    cppq = ppq;
    Clock_Start();
    SendDirectData = SendDirectDataPtr;
    SendDirectLongData = SendDirectLongDataPtr;
    PrepareLongData = PrepareLongDataPtr;
    UnprepareLongData = UnprepareLongDataPtr;
    while(TRUE){
        StartTimeCheck();
        double newClock = Clock_GetTick();
        if(newClock!=clock){
            totalFrames++;
            clock=newClock;
            unsigned long long *tPos = &trackPosition[0];
            cEv = &currEvent[0];
            pStep = &prepareStep[0];
            eT = &eventType[0];
            prevE = &prevEvent[0];
            unsigned long long clockUInt64 = (unsigned long long)clock;
            BOOL *tF1 = &trackFinished[0];
            for(unsigned int i = 0; i < realTracks; i++){
                if(*tF1==FALSE){
                    tR = trackReads[i];
                    unsigned long long tempPos = *tPos;
                    BOOL doloop = TRUE;
                    BOOL doloop2 = TRUE;
                    BOOL tempstep = *pStep;
                    while(doloop2){
                        if(tempstep){
                            unsigned long int event = 0;
                            byte tempPrev = *prevE;
                            while(doloop){
                                unsigned long int val = 0;
                                byte temp = 0;
                                do
                                {
                                    temp = *(tR++);
                                    val = (val << 7) | (temp & 0x7F);
                                } while (temp & 0x80);
                                tempPos+=val;
                                byte readEvent = *(tR++);
                                if (readEvent < 0x80) {
                                    tR--;
                                    readEvent = tempPrev;
                                }
                                tempPrev = readEvent;
                                byte trackEvent = readEvent & 0b11110000;
                                if (trackEvent == 0x90 || trackEvent == 0x80 || trackEvent == 0xA0 || trackEvent == 0xE0 || trackEvent == 0xB0) {
                                    if(tempPos<=clockUInt64){
                                        SendDirectData((readEvent | (*(tR++) << 8) | (*(tR++) << 16)));
                                        sentEvents++;
                                    } else {
                                        *cEv = (readEvent | (*(tR++) << 8) | (*(tR++) << 16));
                                        doloop=FALSE;
                                        tempstep=FALSE;
                                        *eT=0;
                                        break;
                                    }
                                } else if (trackEvent == 0xC0 || trackEvent == 0xD0) {
                                    if(tempPos<=clockUInt64){
                                        SendDirectData((readEvent | (*(tR++) << 8)));
                                        sentEvents++;
                                    } else {
                                        *cEv = (readEvent | (*(tR++) << 8));
                                        doloop=FALSE;
                                        tempstep=FALSE;
                                        *eT=0;
                                        break;
                                    }
                                } else if (readEvent == 0) {
                                    doloop=FALSE;
                                    break;
                                } else {
                                    switch (readEvent) {
                                        case 0b11110000:
                                        {
                                            if(tempPos<=clockUInt64){
                                                handleSysEx(tR);
                                            } else {
                                                doloop = FALSE;
                                                doloop2 = FALSE;
                                                tempstep = FALSE;
                                                *eT=3;
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
                                            readEvent = *(tR++);
                                            switch(readEvent){
                                                case 0x51:
                                                {
                                                    tR++;
                                                    event = 0;
                                                    for (int i = 0; i != 3; i++) {
                                                        byte temp = *(tR++);
                                                        event = (event << 8) | temp;
                                                    }
                                                    if(tempPos<=clockUInt64){
                                                        Clock_SubmitBPM(tempPos,event);
                                                    } else {
                                                        *cEv = event;
                                                        doloop=FALSE;
                                                        doloop2=FALSE;
                                                        tempstep=FALSE;
                                                        *eT=1;
                                                    }
                                                    break;
                                                }
                                                case 0x2F:
                                                {
                                                    doloop=FALSE;
                                                    *tF1=TRUE;
                                                    aliveTracks--;
                                                    break;
                                                }
                                                case 0x01:
                                                case 0x02:
                                                case 0x05:
                                                case 0x06:
                                                case 0x07:
                                                case 0x08:
                                                {
                                                    if(metaAllow[readEvent]){
                                                        if(tempPos<=clockUInt64){
                                                            byte len = *(tR++);
                                                            unsigned char* range = malloc(len+1);
                                                            for(int i = 0; i < len; i++){
                                                                *(range+i)=*(tR++);
                                                            }
                                                            range[len]='\0';
                                                            metaPrint(readEvent);
                                                            for (int i = 0; i < len; i++) {
                                                                printf("%c", range[i]);
                                                            }
                                                            free(range);
                                                        } else {
                                                            doloop=FALSE;
                                                            doloop2=FALSE;
                                                            tempstep=FALSE;
                                                            *eT=2;
                                                        }
                                                    } else {
                                                        tR += *(tR++);
                                                    }
                                                    break;
                                                }
                                                default:
                                                    tR += *(tR++);
                                                    break;
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            *tPos=tempPos;
                            *prevE=tempPrev;
                            if(!doloop){
                                break;
                            }
                        } else {
                            if(tempPos<=clockUInt64){
                                tempstep=TRUE;
                                switch(*eT){
                                    case 0:
                                        SendDirectData(*cEv);
                                        sentEvents++;
                                        break;
                                    case 1:
                                        Clock_SubmitBPM(tempPos,*cEv);
                                        break;
                                    case 2:
                                    {
                                        tR--;
                                        byte metaType = *(tR++);
                                        byte len = *(tR++);
                                        unsigned char* range = malloc(len+1);
                                        for(int i = 0; i < len; i++){
                                            *(range+i)=*(tR++);
                                        }
                                        range[len]='\0';
                                        metaPrint(metaType);
                                        for (int i = 0; i < len; i++) {
                                            printf("%c", range[i]);
                                        }
                                        free(range);
                                        break;
                                    }
                                    case 3:
                                    {
                                        handleSysEx(tR);
                                        break;
                                    }
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
                *tF1++;*tPos++;*pStep++;*cEv++;*prevE++;*eT++;
            }
        } else {
            //usleep(1000);
            signed long long slp = -1;
            NtSleep(&slp);
        }
        if(aliveTracks == 0){
            printf("\nRan out of events, playback finished.");
            sleep(3);
            exit(0);
        }
    }
}
