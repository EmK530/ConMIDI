unsigned int ppq = 0;
unsigned int fakeTracks = 0;
unsigned int format = 0;
unsigned int realTracks = 0;
unsigned long long notes = 0;
struct SynthEvent {
    unsigned long int pos;
    unsigned long int event;
};
struct Tempo {
    float pos;
    unsigned long int offset;
    unsigned long int event;
};
struct SynthEvent **SynthEvents;
struct Tempo **Tempos;
unsigned long int *eventCounts;
unsigned long int *tempoCounts;
