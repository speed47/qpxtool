all: qpxtransport qpxplextor qpxpioneer qpxyamaha qpxscan


#build targets
qpxtransport:
	$(MAKE) -C qpxtransport DIR=qpxtransport

qpxplextor: qpxtransport
	$(MAKE) -C qpxplextor   DIR=qpxplextor

qpxpioneer: qpxtransport
	$(MAKE) -C qpxpioneer   DIR=qpxpioneer

qpxyamaha: qpxtransport
	$(MAKE) -C qpxyamaha 	DIR=qpxyamaha

qpxscan: qpxtransport qpxplextor
	$(MAKE) -C qpxscan   	DIR=qpxscan


#clean targets
clean: qpxtransport-clean qpxplextor-clean qpxpioneer-clean qpxyamaha-clean qpxscan-clean

qpxtransport-clean:
	$(MAKE) -C qpxtransport DIR=qpxtransport clean

qpxplextor-clean:
	$(MAKE) -C qpxplextor   DIR=qpxplextor clean

qpxpioneer-clean:
	$(MAKE) -C qpxpioneer   DIR=qpxpioneer clean

qpxyamaha-clean:
	$(MAKE) -C qpxyamaha 	DIR=qpxyamaha clean

qpxscan-clean:
	$(MAKE) -C qpxscan   	DIR=qpxscan clean


#install targets
install: install-prereq qpxtransport-inst qpxplextor-inst qpxpioneer-inst qpxyamaha-inst qpxscan-inst

install-prereq:
	mkdir -p $(DESTDIR)$(INCDIR)/qpxtool


qpxtransport-inst:
	$(MAKE) -C qpxtransport DIR=qpxtransport install

qpxplextor-inst:
	$(MAKE) -C qpxplextor   DIR=qpxplextor install

qpxpioneer-inst:
	$(MAKE) -C qpxpioneer   DIR=qpxpioneer install

qpxyamaha-inst:
	$(MAKE) -C qpxyamaha 	DIR=qpxyamaha install

qpxscan-inst:
	$(MAKE) -C qpxscan   	DIR=qpxscan install


#uninstall targets
uninstall: qpxtransport-uninst qpxplextor-uninst qpxpioneer-uninst qpxyamaha-uninst qpxscan-uninst
	rm -rf $(DESTDIR)$(INCDIR)/qpxtool

qpxtransport-uninst:
	$(MAKE) -C qpxtransport DIR=qpxtransport uninstall

qpxplextor-uninst:
	$(MAKE) -C qpxplextor   DIR=qpxplextor uninstall

qpxpioneer-uninst:
	$(MAKE) -C qpxpioneer   DIR=qpxpioneer uninstall

qpxyamaha-uninst:
	$(MAKE) -C qpxyamaha 	DIR=qpxyamaha uninstall

qpxscan-uninst:
	$(MAKE) -C qpxscan   	DIR=qpxscan uninstall


.PHONY: all clean install install-prereq uninstall qpxtransport qpxplextor qpxpioneer qpxyamaha qpxscan

