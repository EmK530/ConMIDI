# ConMIDI

ConMIDI is a lightweight console MIDI player, being the successor to [SharpMIDI v2.4.1](https://github.com/EmK530/SharpMIDI/releases/tag/v2.4.1).

Written in C to achieve insane performance. Able to achieve around 100M NPS with OmniMIDI on my Ryzen 7 1700 at 3.75GHz

## Contributions

I am a new C programmer, so there can very well be some bad practices used in this program. If you spot anything, feel free to report it!

## Prerequisites

Recommended use with this MIDI player for performance is the [OmniMIDI](https://github.com/KeppySoftware/OmniMIDI/releases) synth.

If you don't have OmniMIDI, you can choose the "WinMM" device.

## How to build

Get yourself a copy of GCC (preferably TDM-GCC-64) and use this command:

```
gcc ConMIDI.c -O3 -march=native -mtune=native -s -lcomdlg32 -o ConMIDI
```

## Credits

#### Contributors:

[Lurmog](https://github.com/Lurmog) (File dialog support)

#### Zenith-MIDI's [MidiTrack.cs](https://github.com/arduano/Zenith-MIDI/blob/master/BMEngine/MidiTrack.cs) which the loading code is inspired by.
