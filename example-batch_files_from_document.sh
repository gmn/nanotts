#!/bin/bash
# takes a list of integer pairs, eg. [(113,215),(231,330)] and filename. Then
# creates a list of .mp3 outputs from the textfile at filename, from each startline to
# endline. The line numbers are inclusive.

# I have found these settings considerably improve the legibility of the nanotts output; ymmv
speed="0.8"
speed="0.78"
voice="en-US"
volume="0.6"
pitch="1.14"

FILENAME=../documents/2600_MAGAZINE/2600_\ The\ Hacker\ Digest\ -\ Volume\ 35.txt

# Even though it may not look like it, these numbers are in pairs;
# Each pair is a starting line and ending line of a section of text that is to be made into audio.
SECTIONS="204 224 230 264 270 328 334 462 468 594 600 648 654 782 788 892 898 988 994 1006 1012 1048 1054 1124 1130 1160 1166 1194 1202 1222 1228 1254"

HEAD=''
TAIL=''

function run_nanotts() {
    local count=$3
    while [ ${#count} -lt 3 ]; do count=0$count; done
    echo "nanotts --speed $speed --volume $volume --pitch $pitch --voice $voice < <( head -$1 \"${FILENAME}\" | tail -$2 ) -c | lame -r -s 16 -m m -V 0 -b 56 --ta \"2600 Magazine\" --tl \"2600 Vol.35\" - 2600-vol.35-$count.mp3" >/dev/stderr
          nanotts --speed $speed --volume $volume --pitch $pitch --voice $voice < <( head -$1 "${FILENAME}" | tail -$2 ) -c | lame -r -s 16 -m m -V 0 -b 56 --ta "2600 Magazine" --tl "2600 Vol.35" - 2600-vol.35-$count.mp3
}

COUNT=1
for sect in ${SECTIONS}; do
    echo $sect >/dev/stderr
    if [ -z "$TAIL" ]; then
        TAIL=$sect
    else
        HEAD=$sect
        let TAIL="$HEAD-$TAIL+1"

        echo "head -$HEAD "$FILENAME" | tail -$TAIL" >/dev/stderr
              head -$HEAD "$FILENAME" | tail -$TAIL
        echo >/dev/stderr;

        run_nanotts $HEAD $TAIL ${COUNT}

        HEAD=''
        TAIL=''
        let COUNT="$COUNT+1"

        sleep 3
    fi
done
