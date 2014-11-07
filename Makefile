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
	g++ -I. -I./svoxpico -Wall -g ${OPT_FLAG} -c -o mmfile.o src/mmfile.cpp
	g++ -L./svoxpico/.libs mmfile.o nanotts.o svoxpico/.libs/libttspico.a -g ${OPT_FLAG} -o nanotts -lao -ldl -lm

clean:
	rm mmfile.o nanotts.o nanotts
