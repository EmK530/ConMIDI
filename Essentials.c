_Pragma("GCC diagnostic ignored \"-Wimplicit-function-declaration\"");

#include <stdio.h>
#include <time.h>
#include <windows.h>

int intInput(char* text){
    int num;
    printf(text);
    scanf("%d",&num);
    fflush(stdin);
    return num;
}
void removeSymbol(char text[], char symbol, char* clean){
    int i,j;
    for(i = 0, j = 0; i < strlen(text); i++){
        if(text[i] != symbol){
            clean[j++] = text[i];
        }
    }
    clean[j] = '\0';
}
void error(char text[]){
    printf("\n[ERROR] %s\n",text);
    exit(0);
}
char* concat(const char* str1, const char* str2) {
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char* result = malloc(len1 + len2 + 1);
    if (result == NULL) {
        return NULL;
    }
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}
HMODULE ntdll_libHandle;
char ntdllAvailable = 0;
typedef unsigned int(__stdcall *NtDE)(unsigned char, signed long long);
typedef unsigned int(__stdcall *NtQST)(signed long long*);
NtDE NtDelayExecution;
NtQST NtQuerySystemTime;
double getTimeMsec(void) {
    if(ntdllAvailable==1){
        signed long long ret;
        NtQuerySystemTime(&ret);
        return (double)ret/10000;
    } else {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
    }
}
void NtSleep(signed long long t){
    NtDelayExecution(0, t);
}
void setupntdll(){
    if ((ntdll_libHandle = LoadLibrary("ntdll")) == NULL){printf("Could not load ntdll\n\n");return;}
    ntdllAvailable=1;
    if ((NtDelayExecution = (NtDE)GetProcAddress(ntdll_libHandle, "NtDelayExecution")) == NULL){printf("GetProcAddress failed for ntdll NtDelayExecution.\n\n");}
    if ((NtQuerySystemTime = (NtQST)GetProcAddress(ntdll_libHandle, "NtQuerySystemTime")) == NULL){printf("GetProcAddress failed for ntdll NtQuerySystemTime.\n\n");}
}
