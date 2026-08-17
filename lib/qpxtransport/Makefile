SRC  = colors \
	common_functions \
	qpx_transport \
	qpx_mmc \
	qpx_mmc_css \
	qpx_plextor_auth \
	threads \
	sense

C_SRCS = third_party/openbsd/blowfish.c \
	third_party/openwall/md5.c

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
C_OBJS = $(patsubst %.c,%.o,$(C_SRCS))
OBJS = $(patsubst %.cpp,%.o,$(SRCS)) $(C_OBJS)

THIRD_PARTY_CFLAGS = $(filter-out -std=c++17,$(CFLAGS)) -Ithird_party/openbsd -Ithird_party/openwall

VER_MAJOR  = 0
VER_MINOR  = 8
VER_MICRO  = 0

LDLIBS += $(LIBS_HW) $(LIBS_THREAD) $(LIBS_INET)

include ../Makefile.lib

third_party/openbsd/blowfish.o: third_party/openbsd/blowfish.c third_party/openbsd/blf.h
	$(CC) $(THIRD_PARTY_CFLAGS) -c $< -o $@

third_party/openwall/md5.o: third_party/openwall/md5.c third_party/openwall/md5.h
	$(CC) $(THIRD_PARTY_CFLAGS) -UHAVE_OPENSSL -c $< -o $@
