CC=g++
CFLAGS=-c -Wall -I/c/Malcolm/downloads/Boost/boost_1_54_0/boost_1_54_0
LDFLAGS=
SOURCES=main.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=boost_any_example

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE) $(EXECUTABLE).exe

