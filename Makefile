CC := g++
CO := -c -g -std=c++14 -Wall -Wextra -Wwrite-strings -fPIE -DNDEBUG -D_REENTRANT -D_FILE_OFFSET_BITS=64 -O3 -fno-strict-aliasing -I/usr/include/jsoncpp

LN := g++
LO := -pie -Wl,-version-script=src/version

LIBS := -lcurl -ljsoncpp -lfuse

SRCS := $(wildcard src/*.cpp)
OBJS := $(patsubst %.cpp, %.o, $(SRCS))

TARGET := onedrivefs

first: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LN) $(LO) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CO) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
