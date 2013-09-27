CC=g++
CFLAGS=-c -Wall -I../include
LDFLAGS=
SOURCES=main.cpp \
	moving_towards_any_facade.cpp \
	hoisting_out_an_interface.cpp \
	moving_to_generic_calling_mechanism.cpp \
	using_function_traits.cpp \
	splitting_interfaces.cpp \
	splitting_implementation.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=example

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE) $(EXECUTABLE).exe

