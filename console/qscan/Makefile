SRCS = $(patsubst %,%.cpp, qscan)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

BIN = qscan$(BINSUFF)

CXXFLAGS += -I. -I../../lib/include
CFLAGS   += -I. -I../../lib/include
LDLIBS   += -L../../lib/lib -lqpxtransport -lqpxscan -lqpxplextor

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

all:$(BIN)

clean:
	rm -f $(BIN) $(OBJS) *~

install:
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 4755 $(BIN) $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)
.PHONY: all clean install
