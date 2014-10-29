NanoTTS
=======

Speech synthesizer commandline utility that improves pico2wave, included with SVOX PicoTTS 


## Goal 
----- 

Rewrite pico2wave front-end into something vastly more user friendly. 

Ideally, add features to aid automatic parsing of large text-files (50k+ words) into small batches of automatically named .wav or .mp3 files. The goal is to aid in the structured digestion of papers/articles/books, but also to make more versatile for many other speech synth uses as well.


Steps: 
- [x] get a bare-bones working implementation of picotts, sans cruft
- [ ] create a cmdline interface file that exemplifies canonical Unix-isms, like reading from stdin, writing to stdout, versatile command parsing
- [ ] implement cmdline switches that do:
    - [ ] select voice
    - [ ] set voice file directory path
    - [ ] set .wav name '-o file.wav' 
    - [ ] read from text file input
    - [ ] system playback instead of writing WAV output
    - [ ] on both Mac, Linux, multiple Linux sound systems
    - [ ] write PCM/WAV to stdout
    - [ ] print detailed help
    - [ ] slow-down/speed-up speaking rate of voice
    - [ ] bit-rate, frequency, channel, parms for .wav 

Advanced/overkill wish-list features:
- [ ] mp3 output 
- [ ] store base settings in $HOME/.config type file, so you dont have to type language prefs every time
- [ ] advanced feature to carve up large text-file into set of auto-named .mp3
- [ ] search and replace feature, useful for replacing certain problem characters, such as '-' (pico says "hyphen") that can ruin the flow of a book, so replace '-' with ',' which pico interprets instead as a pause.


a work in progress..
