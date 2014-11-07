PROGRAM = nanotts
TARGETS = svoxpico/.libs/libttspico.a

$(PROGRAM): $(TARGETS)
all: $(PROGRAM)

svoxpico/.libs/libttspico.a:
	cd svoxpico; ./autogen.sh && ./configure && make

nanotts:
	g++ -I. -I./svoxpico -g -O2 -c -o nanotts.o src/nanotts.cpp
	g++ -L./svoxpico/.libs nanotts.o svoxpico/.libs/libttspico.a -g -O2 -o nanotts -lao -ldl -lm

clean:
	rm nanotts.o nanotts
