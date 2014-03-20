MAN1DIR = $(DESTDIR)$(MANDIR)/man1
MAN8DIR = $(DESTDIR)$(MANDIR)/man8

all:

clean:

install:
	mkdir -p $(MAN1DIR)
	mkdir -p $(MAN8DIR)
	install -m 0644 f1tattoo.1.gz $(MAN1DIR)
	install -m 0644 pxfw.8.gz $(MAN8DIR)
	install -m 0644 cdvdcontrol.1.gz $(MAN1DIR)
	install -m 0644 qscan.1.gz $(MAN1DIR)
	install -m 0644 qscand.1.gz $(MAN1DIR)
	install -m 0644 qpxtool.1.gz $(MAN1DIR)
	install -m 0644 readdvd.1.gz $(MAN1DIR)

uninstall:
	rm -f $(MAN1DIR)/f1tattoo.1.gz
	rm -f $(MAN8DIR)/pxfw.8.gz
	rm -f $(MAN1DIR)/cdvdcontrol.1.gz
	rm -f $(MAN1DIR)/qscan.1.gz
	rm -f $(MAN1DIR)/qscand.1.gz
	rm -f $(MAN1DIR)/qpxtool.1.gz
	rm -f $(MAN1DIR)/readdvd.1.gz

.PHONY: all clean install uninstall
