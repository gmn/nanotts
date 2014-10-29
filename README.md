nanotts
=======

Improved SVOX PicoTTS speech synthesizer


## Goal 
----- 

Rewrite pico2wave front-end into something vastly more user friendly. 

Ideally, add features to aid automatic parsing of large text-files (50k+ words) into small batches of automatically named .wav or .mp3 files. The goal is to aid in the structured digestion of papers/articles/books, but also to make more versatile for many other speech synth uses as well.


Steps: 
- [x] get a bare-bones working implementation of picotts, sans cruft
- [ ] create a cmdline interface file that exemplifies canonical Unix-isms, like reading from stdin, writing to stdout, versatile command parsing
- [ ] cmdline switches to:
    - [ ] select voice
    - [ ] set voice file directory
    - [ ] read from text file input
    - [ ] send sound to system playback instead of write PCM/WAV
    - [ ] on both Mac & Linux
    - [ ] write PCM/WAV to stdout
    - [ ] slow-down/speed-up speaking rate of voice
    - [ ] bit-rate, frequency params for .wav 

Advanced/Overkill:
- [ ] store base settings in $HOME/.config type file, so you dont have to enter language on cmdline every time
- [ ] advanced feature to carve up large text-file into set of auto-named .mp3
- [ ] search and replace feature, useful for replacing certain problem characters, such as '-', pico says "hyphen", but that can ruin the flow of a book, so replace '-' with ',' , which pico interprets merely as a pause.
- [ ] mp3 output 


a work in progress..
