CXX=g++
CFLAGS=-std=c++17 -g
LIBS=
VPATH=src
OBJ = attotime.o \
common.o \
ee.o \
iop_dma.o \
iop.o \
main.o \
scph10000.o \
scph15000.o \
scph30000.o

all: $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o emotionless $(LIBS)

clean:
	rm *.o && rm emotionless

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<