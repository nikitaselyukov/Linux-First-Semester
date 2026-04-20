CXX := g++
TARGET := zlib_build_demo
SRC := zlib_build_demo.cc
OBJ := zlib_build_demo.o

CPPFLAGS := $(shell pkg-config --cflags zlib)
LDLIBS := $(shell pkg-config --libs zlib)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDLIBS)

$(OBJ): $(SRC) zlib_build_demo.hh
	$(CXX) $(CPPFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)
