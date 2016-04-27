CXX = /usr/bin/g++
FLAGS = -g -std=c++11 -Wall -W -pedantic-errors
OBJECTS = driver.o
default: all
all: driver

SUFFIXES: .cpp
.cpp.o: $<
	${CXX} -c ${FLAGS} $< -o $@
driver: ${OBJECTS}
	${CXX} ${FLAGS} ${OBJECTS} -o $@

clean:  
	-@rm -f core > /dev/null 2>&1
	-@rm -f driver > /dev/null 2>&1
	-@rm -f ${OBJECTS} > /dev/null 2>&1
driver.o: driver.cpp Page.h
