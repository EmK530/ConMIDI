#ifndef ESSENTIALS_H
#define ESSENTIALS_H

int intInput(char* text);
void removeSymbol(char text[], char symbol, char* clean);
void error(char text[]);
char* concat(const char* str1, const char* str2);
double getTimeMsec(void);
void NtSleep(signed long long* t);
void setupntdll();

#endif