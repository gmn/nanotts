PROGRAM = nanotts
LIBRARY_TARGET = svoxpico/.libs/libttspico.a 
OPT_FLAG = -O2
SHELL := /bin/bash

all: $(PROGRAM)

OBJECTS = \
    mmfile.o    \
    nanotts.o   \
    wav.o       \
    player_ao.o 

%.o: ./src/%.cpp
	g++ -I. -I./svoxpico -Wall -g ${OPT_FLAG} -c $^ -o $@ 

svoxpico/.libs/libttspico.a:
	cd svoxpico; ./autogen.sh && ./configure && make

nanotts: $(LIBRARY_TARGET) $(OBJECTS)
	g++ -L./svoxpico/.libs $(OBJECTS) $(LIBRARY_TARGET) -g ${OPT_FLAG} -o nanotts -lao -ldl -lm

clean:
	@for file in $(OBJECTS) nanotts pico2wave; do if [ -f $${file} ]; then rm $${file}; echo rm $${file}; fi; done

clean_all: clean
	cd svoxpico; make clean ; ./clean.sh

pico: $(LIBRARY_TARGET)
	gcc -I. -I./svoxpico -Wall -g ${OPT_FLAG} -c -o pico2wave.o src/pico2wave.c
	gcc -I./svoxpico -Wall -g ${OPT_FLAG} pico2wave.o svoxpico/.libs/libttspico.a -o pico2wave -lm -lpopt

both: nanotts pico
