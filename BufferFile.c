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
    buffer = malloc(bufSize);
    bufRange = fread(buffer, 1, bufSize, midi);
    curSeek += filePos+bufRange;
    fileEnded = (bufRange != bufSize);
}
void UpdateBuffer(){
    if(!fileEnded){
        filePos+=bufPos;
        fseeko(midi,filePos-curSeek,SEEK_CUR);
        bufRange = fread(buffer, 1, bufSize, midi);
        curSeek = filePos+bufRange;
        fileEnded = (bufRange != bufSize);
        bufPos = 0;
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
    UpdateBuffer();
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
