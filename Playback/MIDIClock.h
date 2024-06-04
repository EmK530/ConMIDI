#ifndef MIDICLOCK_H
#define MIDICLOCK_H

#include <windows.h>

extern char* prgTitle;
extern unsigned int cppq;
extern double bpm;

void Clock_Start();
void Clock_SubmitBPM(double pos, unsigned long int b);
double Clock_GetTick();

#endif