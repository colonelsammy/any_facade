CC=g++
CFLAGS=-c -Wall -I../include -I../../Catch/include
LDFLAGS=
SOURCES=main.cpp \
	AnyBasicUnitTests.cpp \
	AnyCallUnitTests.cpp \
	AnyComparisonUnitTests.cpp \
	AnyMultipleInterfacesUnitTests.cpp \
	TypeInfoUnitTests.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=tests

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE) $(EXECUTABLE).exe

