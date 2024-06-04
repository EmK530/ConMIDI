#ifndef SOUND_H
#define SOUND_H

#include <windows.h>

extern void (*SendDirectDataPtr)(unsigned long int a);
extern int (*SendDirectLongDataPtr)(MIDIHDR *a, unsigned int b);
extern int (*PrepareLongDataPtr)(MIDIHDR *a, unsigned int b);
extern int (*UnprepareLongDataPtr)(MIDIHDR *a, unsigned int b);

extern BOOL usable[2];
extern int usables;
void Sound_Setup();
int Sound_Init(int id);

#endif