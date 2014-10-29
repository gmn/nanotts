## Makefile.am -- Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = nanotts
nanotts_SOURCES = \
	bin/nanotts.cpp
nanotts_LDADD = \
	libttspico.la -lm 
nanotts_CFLAGS = -Wall -I lib

