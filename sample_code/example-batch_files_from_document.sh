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

FILENAME=../../documents/2600_MAGAZINE/2600_\ The\ Hacker\ Digest\ -\ Volume\ 35.txt

# Even though it may not look like it, these numbers are in pairs;
# Each pair is a starting line and ending line of a section of text that is to be made into audio.
SECTIONS="204 224 230 264 270 328 334 462 468 594 600 648 654 782 788 892 898 988 994 1006 1012 1048 1054 1124 1130 1160 1166 1194 1202 1222 1228 1254"
#SECTIONS="1260 1300 1306 1374 1380 1542 1548 1582 1588 1632 1638 1652 1658 1688 1694 1748 1754 1832 1838 1862 1868 1950 1956 1990 1998 2126 2132 2164"
#SECTIONS="2170 2236 2242 2312 2318 2342 2348 2384 2390 2452 2458 2568 2576 2630 2636 2676 2682 2698 2704 2840 2846 2874 2880 2906 2912 3046 3052 3112"
#SECTIONS="3120 3144 3150 3172 3178 3204 3210 3300 3306 3472 3478 3518 3524 3548 3554 3602 3608 3634 3640 3678 3684 3740 3746 3884 3890 3926 3932 3972"

HEAD=''
TAIL=''

function run_nanotts() {
    local count=$3
    local title="$4"
    while [ ${#count} -lt 3 ]; do count=0$count; done
    local file="$count-2600_vol.35-$title.mp3"
    echo "nanotts --speed $speed --volume $volume --pitch $pitch --voice $voice < <( head -$1 \"${FILENAME}\" | tail -$2; echo " . . . . . . " ) -c | lame -r -s 16 -m m -V 0 -b 56 --ta \"2600 Magazine\" --tl \"2600 Vol. 35\" --tn $count - \"$file\"" >/dev/stderr
          nanotts --speed $speed --volume $volume --pitch $pitch --voice $voice < <( head -$1 "${FILENAME}" | tail -$2; echo " . . . . . . " ) -c | lame -r -s 16 -m m -V 0 -b 56 --ta "2600 Magazine" --tl "2600 Vol. 35" --tn $count - "$file"
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

        TITLE=`head -$HEAD "$FILENAME" | tail -$TAIL | head -1`

        run_nanotts $HEAD $TAIL ${COUNT} "${TITLE}"

        HEAD=''; TAIL=''
        let COUNT="$COUNT+1"

        sleep 3
    fi
done
