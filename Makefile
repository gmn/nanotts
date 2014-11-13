
PROGRAM = nanotts
PICO_LIBRARY = svoxpico/.libs/libttspico.a 
#OPT_FLAG = -O2
SHELL := /bin/bash

all: $(PROGRAM)

OBJECTS_DIR = objs

OBJECTS = \
    $(OBJECTS_DIR)/mmfile.o    \
    $(OBJECTS_DIR)/nanotts.o   \
    $(OBJECTS_DIR)/wav.o       \
    $(OBJECTS_DIR)/player_ao.o 

$(OBJECTS_DIR)/%.o: ./src/%.cpp 
	g++ -D_PICO_LANG_DIR=\"/usr/share/pico/lang/\" -I. -I./svoxpico -Wall -g $(OPT_FLAG) -c $^ -o $@ 

$(OBJECTS_DIR):
	@[ -d $(OBJECTS_DIR) ] || mkdir $(OBJECTS_DIR)

$(PICO_LIBRARY):
	cd svoxpico; ./autogen.sh && ./configure && make

$(PROGRAM): $(PICO_LIBRARY) $(OBJECTS_DIR) $(OBJECTS)
	g++ -L./svoxpico/.libs $(OBJECTS) $(PICO_LIBRARY) -g $(OPT_FLAG) -o $(PROGRAM) -lao -ldl -lm

clean:
	@for file in $(OBJECTS) $(PROGRAM) pico2wave.o pico2wave; do if [ -f $${file} ]; then rm $${file}; echo rm $${file}; fi; done
	@if [ -d $(OBJECTS_DIR) ]; then rmdir $(OBJECTS_DIR) ; fi

clean_all: clean
	cd svoxpico; make clean ; ./clean.sh

pico: $(PICO_LIBRARY)
	gcc -I. -I./svoxpico -Wall -g $(OPT_FLAG) -c -o pico2wave.o src/pico2wave.c
	gcc -I./svoxpico -Wall -g $(OPT_FLAG) pico2wave.o svoxpico/.libs/libttspico.a -o pico2wave -lm -lpopt

both: $(PROGRAM) pico
