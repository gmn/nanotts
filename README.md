NanoTTS
=======

Speech synthesizer commandline utility that improves pico2wave, included with SVOX PicoTTS

## Update, December 2018

- Cleaned up the interface. All outputs must be explicitly specified now.
- Most inputs are mandatorily specified, all but trailing words. `nanotts "trailing words"` counts as an input, having the same effect as `nanotts -i "trailing words"`.
- Removed the mandatory libao linkage which has caused problems on systems that don't include it in the default distribution packages.
- Changed the playback module to ALSA.
- Alsa linkage is optional. `make noalsa` builds without alsa. WAVE output still functions.
- All outputs can be multiplexed at the same time. You can literally stream the bytes, write a WAVE and playback the stream at the same time. `nanotts -w -p -c ` accomplishes this.

### Planned
- Windows Build


## Goal
-----

Rewrite pico2wave front-end into something more user friendly.

Ideally, add features to aid automatic parsing of large text-files (50k+ words) into small batches of automatically named .wav or .mp3 files. The goal is to aid in the structured digestion of papers/articles/books, but also to make more versatile for many other speech synthesization uses as well.


Steps:
- [x] get a bare-bones working implementation of picotts, sans cruft
- [X] create cmdline file:
- [ ] implement cmdline switches that do:
    - [X] print detailed help (-h, --help)
    - [X] reads WORDS from stdin        (default, if no other input modes detected)
    - [X] reads WORDS from cmdline      (-w <words>)
    - [X] reads WORDS from file         (-f <filename>)
    - [X] writes WAVE to file           (-o <outputname>)
    - [X] silence device pcm playback   (--no-play|-m)
    - [X] cleanup printed output
    - [X] select voice                  (-v <voice>)
    - [X] writes PCM-data to stdout     (-c)
    - [X] set voice files (lingware) path (-l <path>)
    - [X] set speed
    - [X] set volume
    - [X] set pitch
    - [ ] progress meter to stderr
    - [ ] playback keys: spacebar, left+right arrow, ESC, +/- (playback speed)
    - [ ] run through: gprof, valgrind
    - [ ] write man page ; and make install
    - [ ] autonaming func
    - [ ] -q flag to silence output to {stdout, stderr}
    - [ ] catch signals to cancel PCM playback/output cleanly
    - [ ] confirm working on both Mac and Linux
- [ ] extra:
    - [ ] able to read multiple files at once (-files <file1>[file2][file3][..])
    - [ ] limit text input to N lines
    - [ ] bit-rate, frequency, channel, parms for .wav
    - [ ] mp3 output
    - [ ] store base settings in $HOME/.config type file, so you dont have to type language prefs every time
    - [ ] advanced feature to carve up large text-file into set of auto-named .mp3, supporting -p <prefix>
    - [ ] search & replace, useful for replacing certain problem characters, such as '-' (pico says "hyphen") that can ruin the flow of a book, so replace '-' with ',' which pico interprets instead as a pause.

## MP3 PIPE example
```
echo "eenie meany miny moh" | ./nanotts -c | lame -r -s 16 --bitwidth 16 --signed --little-endian -m m -b 32 -h - out.mp3
```

I know what you're thinking--mp3 is a mess. And you would be right to think that. Basically, because it's raw PCM, you have to tell lame exactly what format to expect. But hey, at least right now mp3 is automatable!


email: _greg AT naughton DOT org_
