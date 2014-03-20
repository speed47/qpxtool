SRC  = qpx_scan \
	qpx_scan_algo \
	qpx_writer

HDRS = include/qpx_scan.h \
	include/qpx_scan_plugin_api.h \
	include/qpx_writer.h

LIBN = qpxscan
SRCS = $(patsubst %,%.cpp, $(SRC))
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

VER_MAJOR  = 0
VER_MINOR  = 7
VER_MICRO  = 0

LDLIBS += $(LIBS_DL) -lqpxtransport -lqpxplextor -L../lib $(LIBS_INET)

include ../Makefile.lib
