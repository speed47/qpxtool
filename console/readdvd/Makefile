SRCS = $(patsubst %,%.cpp, deadreader kbhit sectmap reader reader_disc imgwriter dvd_udf)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

BIN = readdvd$(BINSUFF)

CXXFLAGS += -I. -I../../lib/include
CFLAGS   += -I. -I../../lib/include
LDLIBS   += -L../../lib/lib -lqpxtransport $(LIBS_THREAD)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

all:$(BIN)

clean:
	rm -f $(BIN) $(OBJS) *~

install:
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)

.PHONY: all clean install uninstall
