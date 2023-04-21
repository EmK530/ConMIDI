#include <stdio.h>
#include <windows.h>

FILE* midi;
char* buffer;
size_t bufRange = 0;
unsigned long int bufSize = 0;
long long bufPos = 0;
long long filePos = 0;
long long curSeek = 0;
char* pathSave;
BOOL fileEnded = FALSE;

void BufferInit(char path[], unsigned long seek, unsigned int bufSizee){
    bufSize = bufSizee;
    pathSave = path;
    midi = fopen(path, "rb");
    if(midi==NULL){
        perror("Could not open file path");
        return;
    }
    //printf("\nAllocating buffer of size %u...",bufSize);
    buffer = malloc(bufSize);
    //printf("\nReading file to buffer... (%lu)",bufSize);
    bufRange = fread(buffer, 1, bufSize, midi);
    curSeek += filePos+bufRange;
    fileEnded = (bufRange != bufSize);
}
void UpdateBuffer(){
    if(!fileEnded){
        filePos+=bufPos;
        //printf("\nFSEEK %lu",filePos);
        //printf("\nFSEEK %lu",filePos-curSeek);
        //printf("\nCURSEEK %lu",curSeek);
        //printf("\nFTELL %lu",ftell(midi));
        fseeko(midi,filePos-curSeek,SEEK_CUR);
        //printf("\nFTELL %lu",ftell(midi));
        bufRange = fread(buffer, 1, bufSize, midi);
        curSeek = filePos+bufRange;
        //printf("\nCURSEEK %lu",curSeek);
        fileEnded = (bufRange != bufSize);
        //printf("\nReading file to buffer... (%lu)",bufSize);
        if(fileEnded){
            //printf("\nBuffer reached end of file at %lu",filePos);
        }
        bufPos = 0;
    } else {
        //printf("\nBuffer not updating, file ended.");
    }
}
void Seek(long long pos){
    int cond = pos-filePos>=bufRange;
    bufPos = pos-filePos;
    if(cond){
        UpdateBuffer();
    }
}
int Pushback = -1;
void Skip(unsigned long int count){
    for(unsigned long int i = 0; i < count; i++){
        if(Pushback != -1){
            Pushback = -1;
        }
        if(bufPos >= bufRange){
            UpdateBuffer();
        }
        bufPos++;
    }
}
void ResetBuffer(){
    fileEnded = FALSE;
    bufPos = 0;
    filePos = 0;
    curSeek = 0;
    fseeko(midi,0,SEEK_SET);
    UpdateBuffer();
}
void ResizeBuffer(unsigned long int size){
    bufSize = size;
    free(buffer);
    buffer = malloc(size);
    ResetBuffer();
}
unsigned char Read(){
    if(Pushback != -1){
        int temp = Pushback;
        Pushback = -1;
        return temp;
    }
    if(bufPos>=bufRange){
        UpdateBuffer();
    }
    bufPos++;
    return buffer[bufPos-1];
}
unsigned char ReadFast(){
    if(bufPos>=bufRange){
        UpdateBuffer();
    }
    bufPos++;
    return buffer[bufPos-1];
}
/*
unsigned long int ReadInt32(){
    if(bufPos+4>=bufRange){
        UpdateBuffer();
    }
    unsigned int test = 0;
    for(int i = 0; i != 4; i++){
        test = ((test<<8) | ReadFast());
    }
    return test;
}
*/
unsigned char* ReadRange(int size){
    if(bufPos+size>=bufRange){
        UpdateBuffer();
    }
    unsigned char* range = malloc(size+1);
    for(int i = 0; i < size; i++){
        *(range+i)=*(buffer+bufPos+i);
    }
    range[size]='\0';
    bufPos+=size;
    return range;
}