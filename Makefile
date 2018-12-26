default: compile testrun install

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

compile:
	@xxd -i help.txt modules/helptxt.h
	@g++ -std=c++17 -o enigma main.cc -fdata-sections -Wl,--gc-sections

install: compile
	install -d $(DESTDIR)$(PREFIX)/bin/
	install enigma $(DESTDIR)$(PREFIX)/bin/

testrun:
	# This should return nothing:
	echo "testing...\nThe quick brown fox jumps over the lazy dog." | ./enigma -pDOG -wIII,I,V -uC -sAB,YZ | diff - testout.txt
