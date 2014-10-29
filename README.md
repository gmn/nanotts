nanotts
=======

Improved SVOX PicoTTS speech synthesizer

```
Goal: rewrite the pico2wave front-end sample implementation into something vastly more user friendly, and usable, particularly for the automatic parsing of large text-files (50k+ words) into auto-named .wav or .mp3.
```

Steps: 
- [x] get a bare-bones working implementation of picotts, sans cruft
- [ ] create a cmdline interface file that exemplifies canonical Unix-isms, like reading from stdin, writing to stdout, versatile command parsing
- [ ] cmdline switches to 
- [ ] read from text-file input
- [ ] send sound to system playback instead of write PCM/WAV
- [ ] write PCM/WAV to stdout
- [ ] slow-down/speed-up speaking rate of voice
- [ ] take bit-rate, frequency args for .wav 

Advanced/Overkill:
- [ ] store base settings in $HOME/.config type file, so you dont have to enter language on cmdline every time
- [ ] advanced feature to carve up large text-file into set of auto-named .mp3
- [ ] search and replace feature, useful for replacing certain problem characters, such as '-', pico says "hyphen", but that can ruin the flow of a book, so replace '-' with ',' , which pico interprets merely as a pause.
- [ ] mp3 output 


a work in progress..
