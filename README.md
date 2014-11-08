NanoTTS
=======

Speech synthesizer commandline utility that improves pico2wave, included with SVOX PicoTTS 


## Goal 
----- 

Rewrite pico2wave front-end into something vastly more user friendly. 

Ideally, add features to aid automatic parsing of large text-files (50k+ words) into small batches of automatically named .wav or .mp3 files. The goal is to aid in the structured digestion of papers/articles/books, but also to make more versatile for many other speech synthesization uses as well.


Steps: 
- [x] get a bare-bones working implementation of picotts, sans cruft
- [ ] create cmdline file: 
- [ ] implement cmdline switches that do:
    - [X] print detailed help (-h, --help)
    - [X] reads WORDS from stdin        (default, if no other input modes detected)
    - [ ] reads WORDS from cmdline      (-w <words>)
    - [X] reads WORDS from file         (-f <filename>) 
    - [X] writes WAVE to file (-p [prefix], -o [outputname](overrides prefix; both add file-extension if needed))
    - --------
    - [ ] limit text input to N lines
    - [ ] catch signals to cancel PCM output cleanly
    - [ ] cleanup printed output
    - [ ] DONT play on device pcm (--no-play|-m)
    - [ ] writes PCM-data to stdout (-c)
    - [ ] select voice (-v <voice>)
    - [ ] set voice files (lingware) path (-l <path>)
    - [ ] able to read multiple files at once (-files <file1>[file2][file3][..])
    - [ ] works on both Mac and Linux
- [ ] extra:
    - [ ] slow-down/speed-up speaking rate of voice
    - [ ] bit-rate, frequency, channel, parms for .wav 

Advanced/overkill wish-list features:
- [ ] mp3 output 
- [ ] store base settings in $HOME/.config type file, so you dont have to type language prefs every time
- [ ] advanced feature to carve up large text-file into set of auto-named .mp3
- [ ] search and replace feature, useful for replacing certain problem characters, such as '-' (pico says "hyphen") that can ruin the flow of a book, so replace '-' with ',' which pico interprets instead as a pause.


a work in progress..
