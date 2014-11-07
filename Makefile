PROGRAM = nanotts
TARGETS = svoxpico/.libs/libttspico.a
#OPT_FLAG = -O2
OPT_FLAG = 

$(PROGRAM): $(TARGETS) 
all: $(PROGRAM)

svoxpico/.libs/libttspico.a:
	cd svoxpico; ./autogen.sh && ./configure && make

nanotts:
	g++ -I. -I./svoxpico -Wall -g ${OPT_FLAG} -c -o nanotts.o src/nanotts.cpp
	g++ -L./svoxpico/.libs nanotts.o svoxpico/.libs/libttspico.a -g ${OPT_FLAG} -o nanotts -lao -ldl -lm

clean:
	rm nanotts.o nanotts
