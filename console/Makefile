all: readdvd cdvdcontrol pxfw f1tattoo qscan qscand

clean install uninstall: 
	$(MAKE) -C readdvd     DIR=readdvd $@
	$(MAKE) -C cdvdcontrol DIR=cdvdcontrol $@
	$(MAKE) -C pxfw        DIR=pxfw $@
	$(MAKE) -C f1tattoo    DIR=f1tattoo $@
	$(MAKE) -C qscan       DIR=qscan $@
	$(MAKE) -C qscand      DIR=qscand $@


readdvd:
	$(MAKE) -C readdvd     DIR=readdvd

cdvdcontrol:
	$(MAKE) -C cdvdcontrol DIR=cdvdcontrol

pxfw:
	$(MAKE) -C pxfw        DIR=pxfw

f1tattoo:
	$(MAKE) -C f1tattoo    DIR=f1tattoo

qscan:
	$(MAKE) -C qscan       DIR=qscan

qscand:
	$(MAKE) -C qscand      DIR=qscand

.PHONY: all clean install uninstall readdvd cdvdcontrol pxfw f1tattoo qscan qscand
