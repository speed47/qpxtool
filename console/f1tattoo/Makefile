SRCS = $(patsubst %,%.cpp, f1tattoo)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

BIN = f1tattoo$(BINSUFF)

CXXFLAGS += -I. -I../../lib/include $(LPNG_INC)
CFLAGS   += -I. -I../../lib/include $(LPNG_INC)
LDLIBS   += -L../../lib/lib -lqpxtransport -lqpxyamaha $(LPNG_LIB)

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

.PHONY: all clean install uninstall
