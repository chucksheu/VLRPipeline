# Makefile ComputeMAP

CXXFLAGS = -O2 $(GLOBAL_CXXFLAGS) $(DEBUGFLAGS) -fmessage-length=0 -std=c++11
LDFLAGS = $(GLOBAL_LDFLAGS) -L../lib/

# Common
CXXFLAGS += -I../Common/include/
LDFLAGS += -lcommon

# OpenCV (this goes last: beware of the linking order)
CXXFLAGS += `pkg-config opencv --cflags`
LDFLAGS += `pkg-config opencv --libs`

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

BIN = ComputeMAP

#MAPVERBOSE = -DCMAPVERBOSE

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(BIN) $(LDFLAGS)

.cpp.o:
	$(CXX) $(MAPVERBOSE) -c $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(BIN) *~
