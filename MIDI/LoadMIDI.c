#pragma GCC diagnostic ignored "-Wunused-result"

#include <string.h>
#include "../BufferFile.c"
#include "DataStorage.c"
#include "../Playback/MainPlayer.c"

unsigned long int *trackPositions;
unsigned long int *trackSizes;

int TextSearch(char text[]){
    unsigned char* str = ReadRange(strlen(text));
    int res = strcmp(str,text);
    free(str);
    return 1-res;
}

int IndexTrack(){
    if(TextSearch("MTrk")==1){
        unsigned long int size = (ReadFast()*16777216)+(ReadFast()*65536)+(ReadFast()*256)+ReadFast();
        trackPositions[realTracks]=filePos+bufPos;
        trackSizes[realTracks]=size;
        //printf("\nTrack %u | Size %lu",realTracks+1,size);
        bufPos+=size;
        return 1;
    } else {
        printf("\nTextSearch failed at %lu",filePos+bufPos);
        return 0;
    }
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
void ParseTrack(unsigned long int id, int thres, unsigned long int size){
    unsigned long int skippedNotes[16][256];
    memset(skippedNotes,0,sizeof(skippedNotes));
    byte prevEvent = 0;
    Seek(trackPositions[id]);
    SynthEvents[id] = malloc(size * sizeof(struct SynthEvent));
    Tempos[id] = malloc(size * sizeof(struct SynthEvent));
    eventCounts[id] = 0;
    tempoCounts[id] = 0;
    float trackTime = 0;
    unsigned long int idx = 0;
    unsigned long int idx2 = 0;
    unsigned long int start = filePos+bufPos;
    while((filePos+bufPos)-start<size){
        trackTime += ReadVariableLen();
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
                            struct SynthEvent eventToAdd = { trackTime, readEvent | (note << 8) | (vel << 16) };
                            SynthEvents[id][idx]=eventToAdd;
                            idx++;
                            eventCounts[id]++;
                        } else {
                            skippedNotes[ch][note]++;
                        }
                    } else {
                        if(skippedNotes[ch][note]==0){
                            struct SynthEvent eventToAdd = { trackTime, readEvent | (note << 8) | (vel << 16) };
                            SynthEvents[id][idx]=eventToAdd;
                            idx++;
                            eventCounts[id]++;
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
                        struct SynthEvent eventToAdd = { trackTime, readEvent | (note << 8) | (vel << 16) };
                        SynthEvents[id][idx]=eventToAdd;
                        idx++;
                        eventCounts[id]++;
                    } else {
                        skippedNotes[ch][note]--;
                    }
                }
                break;
            case 0b10100000:
                {
                    byte note = Read();
                    byte vel = Read();
                    struct SynthEvent eventToAdd = { trackTime, readEvent | (note << 8) | (vel << 16) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                }
                break;
            case 0b11000000:
                {
                    byte program = Read();
                    struct SynthEvent eventToAdd = { trackTime, readEvent | (program << 8) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                }
                break;
            case 0b11010000:
                {
                    byte pressure = Read();
                    struct SynthEvent eventToAdd = { trackTime, readEvent | (pressure << 8) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                }
                break;
            case 0b11100000:
                {
                    byte l = Read();
                    byte m = Read();
                    struct SynthEvent eventToAdd = { trackTime, readEvent | (l << 8) | (m << 16) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
                }
                break;
            case 0b10110000:
                {
                    byte cc = Read();
                    byte vv = Read();
                    struct SynthEvent eventToAdd = { trackTime, readEvent | (cc << 8) | (vv << 16) };
                    SynthEvents[id][idx]=eventToAdd;
                    idx++;
                    eventCounts[id]++;
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
                            //printf("\nAdding tempo %lu",tempo);
                            struct SynthEvent eventToAdd2 = { trackTime, tempo };
                            Tempos[id][idx2]=eventToAdd2;
                            idx2++;
                            tempoCounts[id]++;
                            //printf("\nAdded tempo %lu",tempo);
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
}

void LoadMIDI(char path[], int thres, unsigned int bs){
    BufferInit(path, 0, bs);
    trackPositions = malloc(65535 * sizeof(unsigned long int));
    trackSizes = malloc(65535 * sizeof(unsigned long int));
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
    printf("\nIndexing tracks...");
    while(realTracks<fakeTracks){
        if(IndexTrack()==1){
            realTracks++;
        } else {
            printf("\nIndexing done");
            break;
        }
    }
    realloc(trackPositions, realTracks * sizeof(unsigned long int));
    realloc(trackSizes, realTracks * sizeof(unsigned long int));
    eventCounts = malloc(realTracks * sizeof(unsigned long int));
    tempoCounts = malloc(realTracks * sizeof(unsigned long int));
    ResizeBuffer(100000000);
    printf("\nIndexed %d tracks",realTracks);
    printf("\nBegin parsing...");
    SynthEvents = malloc(realTracks * sizeof(struct SynthEvent *));
    Tempos = malloc(realTracks * sizeof(struct SynthEvent *));
    for(int i = 0; i < realTracks; i++){
        printf("\nTrack %hu / %hu | Size %lu",i+1,realTracks,trackSizes[i]);
        ParseTrack(i,thres,trackSizes[i]);
    }
    printf("\nLoaded %lu notes.",notes);
    free(trackPositions);
    free(trackSizes);
    free(buffer);
    printf("\nBeginning playback...");
    StartPlayback();
}