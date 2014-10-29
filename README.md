nanotts
=======

Improved SVOX PicoTTS speech synthesizer

Goal: rewrite the pico2wave front-end sample implementation into something vastly more user friendly, and usable, particularly for the automatic parsing of large text-files (50k+ words) into auto-named .wav or .mp3.

Steps: 
1 - get a bare-bones implementation of picotts working, sans cruft
2 - create a cmdline interface file that obeys canonical Unix-isms, like reading from stdin, writing to stdout
3 - cmdline switch to read from text-file input
4 - be able to send sound directly to system instead of write PCM/WAV
5 - write PCM/WAV to stdout
6 - store base settings in $HOME/.config type file, so you dont have to enter language on cmdline every time
7 - advanced feature to carve up large text-file into set of auto-named *.mp3
8 - search and replace feature, useful for replacing certain problem characters, such as '-', pico says "hyphen", but that can ruin the flow of a book, so replace '-' with ','
9 - mp3 output ?
10 - ability to slow-down/speed-up speaking rate of voice
11 - ability to take user-supplied bit-rate and frequency arguments for the .wav file (right now freq is set at 16000Hz)

..work in progress..
