all: generic plextor pioneer nec liteon asus benq benq_dvdrom tsst

clean install uninstall:
	$(MAKE) -C generic DIR=generic $@
	$(MAKE) -C plextor DIR=plextor $@
	$(MAKE) -C pioneer DIR=pioneer $@
	$(MAKE) -C nec     DIR=nec $@
	$(MAKE) -C liteon  DIR=liteon $@
	$(MAKE) -C asus    DIR=asus $@
	$(MAKE) -C benq    DIR=benq $@
	$(MAKE) -C benq_dvdrom DIR=benq_dvdrom $@
	$(MAKE) -C tsst    DIR=tsst $@

generic:
	$(MAKE) -C generic DIR=generic
plextor:
	$(MAKE) -C plextor DIR=plextor
pioneer:
	$(MAKE) -C pioneer DIR=pioneer
nec:
	$(MAKE) -C nec     DIR=nec
liteon:
	$(MAKE) -C liteon  DIR=liteon
asus:
	$(MAKE) -C asus    DIR=asus
benq:
	$(MAKE) -C benq    DIR=benq
benq_dvdrom:
	$(MAKE) -C benq_dvdrom DIR=benq_dvdrom
tsst:
	$(MAKE) -C tsst    DIR=tsst


.PHONY: all clean install uninstall generic plextor pioneer nec liteon asus benq benq_dvdrom tsst
