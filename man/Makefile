MAN1DIR = $(DESTDIR)$(MANDIR)/man1
MAN8DIR = $(DESTDIR)$(MANDIR)/man8

all: f1tattoo.1.gz pxfw.8.gz cdvdcontrol.1.gz qscan.1.gz qscand.1.gz qpxtool.1.gz readdvd.1.gz

f1tattoo.1.gz:
	gzip -c f1tattoo.1 > f1tattoo.1.gz

pxfw.8.gz:
	gzip -c pxfw.8 > pxfw.8.gz

cdvdcontrol.1.gz:
	gzip -c cdvdcontrol.1 > cdvdcontrol.1.gz

qscan.1.gz:
	gzip -c qscan.1 > qscan.1.gz

qscand.1.gz:
	gzip -c qscand.1 > qscand.1.gz

qpxtool.1.gz:
	gzip -c qpxtool.1 > qpxtool.1.gz

readdvd.1.gz:
	gzip -c readdvd.1 > readdvd.1.gz

clean:
	rm -f f1tattoo.1.gz
	rm -f pxfw.8.gz
	rm -f cdvdcontrol.1.gz
	rm -f qscan.1.gz
	rm -f qscand.1.gz
	rm -f qpxtool.1.gz
	rm -f readdvd.1.gz

install: f1tattoo.1.gz pxfw.8.gz cdvdcontrol.1.gz qscan.1.gz qscand.1.gz qpxtool.1.gz readdvd.1.gz
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
