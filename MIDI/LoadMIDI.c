#pragma GCC diagnostic ignored "-Wunused-result"

#include <string.h>
#include "../BufferFile.c"
#include "DataStorage.c"
#include "../Playback/MainPlayer.c"

long long int lastPos = 0;
unsigned long int lastSize = 0;

int TextSearch(char text[]){
    unsigned char* str = ReadRange(strlen(text));
    int res = strcmp(str,text);
    free(str);
    return 1-res;
}
long long ReadVariableLen(){
    byte temp;
    long long val = 0;
    for(int i = 0; i < 4; i++){
        temp = ReadFast();
        if(temp > 0x7F){
            val = (val << 7) | (temp & 0x7F);
        } else {
            val = val << 7 | temp;
            return val;
        }
    }
    return val;
}
char ParseTrack(unsigned long int id, int thres){
    unsigned long int skippedNotes[16][256];
    memset(skippedNotes,0,sizeof(skippedNotes));
    byte prevEvent = 0;
    if(lastPos!=0){
        Seek(lastPos+lastSize+8);
    }
    lastPos=filePos+bufPos;
    if(TextSearch("MTrk")!=1){
        printf("\nTextSearch failed at %lu",filePos+bufPos);
        return 1;
    }
    lastSize = (ReadFast()*16777216)+(ReadFast()*65536)+(ReadFast()*256)+ReadFast();
    printf("\nTrack %hu / %hu | Size %lu",id+1,fakeTracks,lastSize);
    SynthEvents[id] = malloc(lastSize/3 * sizeof(struct SynthEvent));
    Tempos[id] = malloc(lastSize/6 * sizeof(struct Tempo));
    eventCounts[id] = 0;
    tempoCounts[id] = 0;
    float trackTime = 0;
    unsigned long int idx = 0;
    unsigned long int idx2 = 0;
    unsigned long int offset = 0;
    while((filePos+bufPos)-lastPos<lastSize){
        unsigned long int temp = ReadVariableLen();
        offset += temp;
        trackTime += temp;
        byte readEvent = ReadFast();
        if(readEvent < 0x80){
            Pushback = readEvent;
            readEvent = prevEvent;
        }
        prevEvent = readEvent;
        byte trackEvent = readEvent & 0b11110000;
        switch(trackEvent){
            case 0b10010000:
                {
                    byte ch = readEvent&0b00001111;
                    byte note = Read();
                    byte vel = ReadFast();
                    if(vel != 0){
                        if(vel>=thres){
                            notes++;
                            struct SynthEvent eventToAdd = { offset, readEvent | (note << 8) | (vel << 16) };
                            SynthEvents[id][idx]=eventToAdd;
                            idx++;
                            eventCounts[id]++;
                            offset=0;
                        } else {
                            skippedNotes[ch][note]++;
                        }
                    } else {
                        if(skippedNotes[ch][note]==0){
                            struct SynthEvent eventToAdd = { offset, readEvent | (note << 8) | (vel << 16) };
                            SynthEvents[id][idx]=eventToAdd;
                            idx++;
                            eventCounts[id]++;
                            offset=0;
                        } else {
                            skippedNotes[ch][note]--;
                        }
                    }
                }
                break;
            case 0b10000000:
                {
                    byte ch = readEvent&0b00001111;
                    byte note = Read();
                    byte vel = ReadFast();
                    if(skippedNotes[ch][note]==0){
                        struct SynthEvent eventToAdd = { offset, readEvent | (note << 8) | (vel << 16) };
                        SynthEvents[id][idx]=eventToAdd;
                        idx++;
                        eventCounts[id]++;
                        offset=0;
                    } else {
                        skippedNotes[ch][note]--;
                    }
                }
                break;
            case 0b10100000:
                {
                    byte note = Read();
                    byte vel = Read();
                    struct SynthEvent eventToAdd = { offset, readEvent | (note << 8) | (vel << 16) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                    offset=0;
                }
                break;
            case 0b11000000:
                {
                    byte program = Read();
                    struct SynthEvent eventToAdd = { offset, readEvent | (program << 8) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                    offset=0;
                }
                break;
            case 0b11010000:
                {
                    byte pressure = Read();
                    struct SynthEvent eventToAdd = { offset, readEvent | (pressure << 8) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                    offset=0;
                }
                break;
            case 0b11100000:
                {
                    byte l = Read();
                    byte m = Read();
                    struct SynthEvent eventToAdd = { offset, readEvent | (l << 8) | (m << 16) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                    offset=0;
                }
                break;
            case 0b10110000:
                {
                    byte cc = Read();
                    byte vv = Read();
                    struct SynthEvent eventToAdd = { offset, readEvent | (cc << 8) | (vv << 16) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                    offset=0;
                }
                break;
            case 0:
                break;
            default:
                switch(readEvent){
                    case 0b11110000:
                        while(Read()!=0b11110111);
                        break;
                    case 0b11110010:
                        Skip(2);
                        break;
                    case 0b11110011:
                        Skip(1);
                        break;
                    case 0xFF:
                        readEvent = Read();
                        if(readEvent == 81){
                            Skip(1);
                            unsigned long int tempo = 0;
                            for (int i = 0; i != 3; i++){
                                byte temp = Read();
                                tempo = (tempo<<8)|temp;
                            }
                            struct Tempo eventToAdd2 = { trackTime, tempo };
                            Tempos[id][idx2]=eventToAdd2;
                            idx2++;
                            tempoCounts[id]++;
                        } else if(readEvent == 0x2F){
                            break;
                        } else {
                            Skip(Read());
                        }
                        break;
                    default:
                        break;
                }
                break;
        }
    }
    realloc(SynthEvents[id], idx * sizeof(struct SynthEvent));
    realloc(Tempos[id], idx2 * sizeof(struct SynthEvent));
    return 0;
}

void LoadMIDI(char path[], int thres, unsigned int bs){
    BufferInit(path, 0, bs);
    if(TextSearch("MThd")==0){
        error("MIDI header not found");
    }
    Skip(4);
    format = ReadFast()*256+ReadFast();
    fakeTracks = ReadFast()*256+ReadFast();
    ppq = ReadFast()*256+ReadFast();
    printf("\nFormat: %d",format);
    printf("\nExpected Track Count: %d",fakeTracks);
    printf("\nPPQ: %d",ppq);
    eventCounts = malloc(fakeTracks * sizeof(unsigned long int));
    tempoCounts = malloc(fakeTracks * sizeof(unsigned long int));
    ResizeBuffer(100000000);
    printf("\nBegin parsing...");
    SynthEvents = malloc(fakeTracks * sizeof(struct SynthEvent *));
    Tempos = malloc(fakeTracks * sizeof(struct SynthEvent *));
    int i = 0;
    for(i = 0; i < fakeTracks; i++){
        if(ParseTrack(i,thres)==1){
            break;
        }
    }
    realTracks=i;
    printf("\nLoaded %d tracks",realTracks);
    SynthEvents = realloc(SynthEvents, realTracks * sizeof(struct SynthEvent));
    Tempos = realloc(Tempos, realTracks * sizeof(struct SynthEvent));
    eventCounts = realloc(eventCounts, realTracks * sizeof(unsigned long int));
    tempoCounts = realloc(tempoCounts, realTracks * sizeof(unsigned long int));
    printf("\nLoaded %lu notes.",notes);
    free(buffer);
    printf("\nBeginning playback...");
    StartPlayback();
}
