NanoTTS
=======

Speech synthesizer commandline utility that improves pico2wave, included with SVOX PicoTTS 


## Goal 
----- 

Rewrite pico2wave front-end into something vastly more user friendly. 

Ideally, add features to aid automatic parsing of large text-files (50k+ words) into small batches of automatically named .wav or .mp3 files. The goal is to aid in the structured digestion of papers/articles/books, but also to make more versatile for many other speech synth uses as well.


Steps: 
- [x] get a bare-bones working implementation of picotts, sans cruft
- [ ] create cmdline file: 
    - [ ] reads WORDS from stdin, plays WAVE on device pcm (default)
    - [ ] reads WORDS from cmdline (-w <words>), writes WAVE to file (-o [optional-name])
    - [ ] reads WORDS from file (-f <filename>), writes WAVE to stdout (-c)
- [ ] implement cmdline switches that do:
    - [ ] select voice (-v <voice>)
    - [ ] set voice file directory path (-p <path>)
    - [ ] works on both Mac and Linux
    - [ ] print detailed help (-h, --help)
- [ ] extra:
    - [ ] slow-down/speed-up speaking rate of voice
    - [ ] bit-rate, frequency, channel, parms for .wav 

Advanced/overkill wish-list features:
- [ ] mp3 output 
- [ ] store base settings in $HOME/.config type file, so you dont have to type language prefs every time
- [ ] advanced feature to carve up large text-file into set of auto-named .mp3
- [ ] search and replace feature, useful for replacing certain problem characters, such as '-' (pico says "hyphen") that can ruin the flow of a book, so replace '-' with ',' which pico interprets instead as a pause.


a work in progress..
