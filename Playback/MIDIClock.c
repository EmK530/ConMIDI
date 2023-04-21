#include <windows.h>
#include <time.h>

float timee = 0;
unsigned int cppq = 0;
double bpm = 120;
double ticklen = 0;
double last = 0;
BOOL throttle = TRUE;
double timeLost = 0;
long long startTime = 0;
void Clock_Start(){
    startTime = timeInMilliseconds();
    ticklen = ((double)1 / (double)cppq) * ((double)60 / bpm);
}
void Clock_Reset(){
    startTime = timeInMilliseconds();
    timee = 0;
    last = 0;
    timeLost = 0;
}
double Clock_GetPassedTime(){
    return (double)(timeInMilliseconds()-startTime)/1000;
}
double Clock_GetElapsed(){
    double temp = ((double)Clock_GetPassedTime());
    if(throttle==TRUE){
        if(temp-last>(double)0.0166666){
            timeLost += (temp - last) - (double)0.0166666;
            last = temp;
            return temp-timeLost;
        }
    }
    last = temp;
    return temp-timeLost;
}
void Clock_SubmitBPM(double pos, unsigned long int b){
    double remainder = (timee - pos);
    timee = pos + (Clock_GetElapsed()/ticklen);
    bpm = 60000000 / b;
    //printf("\nNew BPM: %f",bpm);
    timeLost = 0;
    ticklen = ((double)1 / (double)cppq) * ((double)60 / bpm);
    timee += remainder;
    startTime = timeInMilliseconds();
}
double Clock_GetTick(){
    return timee + (Clock_GetElapsed() / ticklen);
}
