
PROGRAM = nanotts
PICO_LIBRARY = svoxpico/.libs/libttspico.a
CFLAGS = -Wall
CFLAGS_DEBUG = -g
CFLAGS_OPT = -O2
SHELL := /bin/bash
PICO_LANG_ROOT := /usr/share/pico
PICO_LANG_LOCATION := $(PICO_LANG_ROOT)/lang/

#LINKER_FLAGS := -lasound -lao
LINKER_FLAGS := -lasound -lm

all: $(PROGRAM)

OBJECTS_DIR = objs

#    $(OBJECTS_DIR)/player_ao.o                  \

OBJECTS = \
    $(OBJECTS_DIR)/mmfile.o                     \
    $(OBJECTS_DIR)/main.o                       \
    $(OBJECTS_DIR)/wav.o                        \
    $(OBJECTS_DIR)/lowest_file_number.o         \
    $(OBJECTS_DIR)/StreamHandler.o              \



ALSA := $(OBJECTS_DIR)/Player_Alsa.o


ifeq ($(MAKECMDGOALS),debug)
    override CFLAGS += $(CFLAGS_DEBUG)
else
    override CFLAGS += $(CFLAGS_OPT)
endif


$(ALSA):
	g++ $(CFLAGS) -c -o $(ALSA) src/Player_Alsa.cpp -lasound

$(OBJECTS_DIR)/%.o: ./src/%.cpp
	g++ -I. -I./svoxpico $(CFLAGS) -c $^ -o $@

$(OBJECTS_DIR):
	@[ -d $(OBJECTS_DIR) ] || mkdir $(OBJECTS_DIR)

$(PICO_LIBRARY):
	cd svoxpico; ./autogen.sh && ./configure && make

$(PROGRAM): $(PICO_LIBRARY) $(OBJECTS_DIR) $(ALSA) $(OBJECTS)
	g++ -L./svoxpico/.libs $(ALSA) $(OBJECTS) $(PICO_LIBRARY) $(CFLAGS) -o $(PROGRAM) $(LINKER_FLAGS)

debug: $(PICO_LIBRARY) $(OBJECTS_DIR) $(ALSA) $(OBJECTS)
	g++ -L./svoxpico/.libs $(ALSA) $(OBJECTS) $(PICO_LIBRARY) $(CFLAGS) -o $(PROGRAM) $(LINKER_FLAGS)

clean:
	@for file in $(ALSA) $(OBJECTS) $(PROGRAM) pico2wave.o pico2wave; do if [ -f $${file} ]; then rm $${file}; echo rm $${file}; fi; done
	@if [ -d $(OBJECTS_DIR) ]; then rmdir $(OBJECTS_DIR) ; fi
	@echo "use \"make distclean\" to also cleanup svoxpico directory"

distclean: clean
	cd svoxpico; make clean ; ./clean.sh

pico: $(PICO_LIBRARY)
	gcc -I. -I./svoxpico -Wall -g $(OPT_FLAG) -c -o pico2wave.o src/pico2wave.c
	gcc -I./svoxpico -Wall -g $(OPT_FLAG) pico2wave.o svoxpico/.libs/libttspico.a -o pico2wave -lm -lpopt

both: $(PROGRAM) pico

install: $(PROGRAM)
	install -m 0755 $(PROGRAM) /usr/bin/
	@if [ ! -d $(PICO_LANG_LOCATION) ]; then echo mkdir -p -m 777 $(PICO_LANG_LOCATION); mkdir -p -m 777 $(PICO_LANG_LOCATION); fi
	@for file in ./lang/* ; do echo install -m 0644 $${file} $(PICO_LANG_LOCATION); install -m 0644 $${file} $(PICO_LANG_LOCATION); done

uninstall:
	@if [ -e /usr/bin/$(PROGRAM) ]; then echo rm /usr/bin/$(PROGRAM); rm /usr/bin/$(PROGRAM); fi
	@if [ -e $(PICO_LANG_ROOT) ]; then echo rm -rf $(PICO_LANG_ROOT); rm -rf $(PICO_LANG_ROOT) ; fi

