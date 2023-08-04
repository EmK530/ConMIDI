# ConMIDI-rawstore

This branch modifies the code to load the MIDI tracks directly into memory and parse events during playback

This speeds up loading, lowers memory usage by double but slows down the playback performance.

## Warning

This branch is semi-functional, playback works but there's a subtle parsing issue that could ruin some MIDIs. If you can figure this out I would be very grateful.

## Contributions

There might be some possible room for optimizing this direct parsing, I simply just threw a simple working branch together.

If you can come up with any improvements, feel free to help out!
