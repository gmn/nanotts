PROGRAM = nanotts
TARGETS = pico2wave/.libs/libttspico.so 

$(PROGRAM): $(TARGETS)
all: $(PROGRAM)

pico2wave/.libs/libttspico.so:
	cd svoxpico; ./autogen.sh && ./configure && make

nanotts:
	g++ -I. -Isvoxpico -g -O2 -c -o nanotts.o src/nanotts.cpp
	g++ nanotts.o pico2wave/.libs/libttspico.so -g -O2 -o nanotts -lm

clean:
	rm nanotts.o nanotts
