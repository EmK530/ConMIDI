# ConMIDI

ConMIDI is the successor to [SharpMIDI v2.4.1](https://github.com/EmK530/SharpMIDI/releases/tag/v2.4.1). Written in C to create a very lightweight console app with way better performance.

## Contributions

I am a new C programmer, so there can very well be some bad practices used in this program. If you spot anything, feel free to report it!

## Prerequisites

This app is currently not finalized yet, so it is solely developed for use with: [OmniMIDI](https://github.com/KeppySoftware/OmniMIDI/releases) and XSynth.

Like with SharpMIDI, you can use a XSynth DLL named "XSynth.dll" however OmniMIDI is recommended.

## How to build

Get yourself a copy of GCC and use this command:

```
gcc ConMIDI.c -Ofast -s -ffast-math -lcomdlg32 -o ConMIDI
```

## Credits

#### Contributors: @Lurmog

#### Zenith-MIDI's [MidiTrack.cs](https://github.com/arduano/Zenith-MIDI/blob/master/BMEngine/MidiTrack.cs) which the loading code is inspired by.
