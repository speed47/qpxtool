SRCS = $(patsubst %,%.cpp, pxfw)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

BIN = pxfw$(BINSUFF)

CXXFLAGS += -I. -I../../lib/include
CFLAGS   += -I. -I../../lib/include
LDLIBS   += -L../../lib/lib -lqpxtransport -lqpxplextor

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

all:$(BIN)

clean:
	rm -f $(BIN) $(OBJS) *~

install:
	mkdir -p $(DESTDIR)$(SBINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(SBINDIR)

uninstall:
	rm -f $(DESTDIR)$(SBINDIR)/$(BIN)

.PHONY: all clean install uninstall
