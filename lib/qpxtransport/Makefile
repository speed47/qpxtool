SRC  = common_functions \
	qpx_transport \
	qpx_mmc \
	qpx_mmc_css \
	threads \
	sense

HDRS = include/qpx_mmc.h \
	include/qpx_mmc_defs.h \
	include/qpx_opcodes.h \
	include/qpx_transport.h \
	include/common_functions.h \
	include/threads.h \
	include/sense.h \
	include/csstables.h \
	include/colors.h

LIBN = qpxtransport
SRCS = $(patsubst %,%.cpp, $(SRC))
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

VER_MAJOR  = 0
VER_MINOR  = 7
VER_MICRO  = 0

LDLIBS += $(LIBS_HW) $(LIBS_THREAD) $(LIBS_INET)

include ../Makefile.lib
