unsigned int ppq = 0;
unsigned int fakeTracks = 0;
unsigned int format = 0;
unsigned int realTracks = 0;
unsigned long long notes = 0;
struct SynthEvent {
    float pos;
    unsigned long int event;
};
struct SynthEvent **SynthEvents;
struct SynthEvent **Tempos;
unsigned long int *eventCounts;
unsigned long int *tempoCounts;
