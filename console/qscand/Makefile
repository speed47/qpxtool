SRCS = $(patsubst %,%.cpp, qscand child device_policy)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

BIN = qscand$(BINSUFF)

CXXFLAGS += -I. -I../../lib/include
CFLAGS   += -I. -I../../lib/include
LDLIBS   += -L../../lib/lib -lqpxtransport

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS) $(LIBS_THREAD) $(LIBS_INET)

qscand.o child.o device_policy.o: device_policy.h

all:$(BIN)

clean:
	rm -f $(BIN) $(OBJS) *~

install:
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)
.PHONY: all clean install
