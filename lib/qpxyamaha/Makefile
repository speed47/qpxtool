SRC  = yamaha_features
HDRS = include/yamaha_features.h
LIBN = qpxyamaha
SRCS = $(patsubst %,%.cpp, $(SRC))
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

VER_MAJOR  = 0
VER_MINOR  = 7
VER_MICRO  = 0

LDLIBS += -lqpxtransport -L../lib

include ../Makefile.lib

