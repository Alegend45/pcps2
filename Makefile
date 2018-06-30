CXX=g++
CFLAGS=-std=c++14 -g
LIBS=
VPATH=src
OBJ = ee.o \
iop.o \
main.o \
scph10000.o \
scph15000.o

all: $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o emotionless $(LIBS)

clean:
	rm *.o && rm emotionless

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<