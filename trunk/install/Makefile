#
# Create installation files for tMan
# Copyright (c) 2008, David Andrs
#

PRODUCT=tMan
VERSION=0.9.11

#CABWIZ="c:/program files/windows ce tools/wce420/pocket pc 2003/tools/cabwiz.exe"
CABWIZ=/opt/wince-arm/bin/cabwiz

NSIS="c:/program files/nsis/makensis.exe"
ZIP="c:/program files/unxutils/usr/local/wbin/zip.exe"

all: cab exe

cab: $(PRODUCT).ARM.CAB

exe: $(PRODUCT)-$(VERSION).exe

$(PRODUCT).ARM.CAB: tman.inf
	$(CABWIZ) tMan.inf /cpu ARM

zip: $(PRODUCT).ARM.CAB
	cp $(PRODUCT).ARM.CAB $(PRODUCT)-$(VERSION).ARM.cab
	$(ZIP) -m $(PRODUCT)-$(VERSION).zip $(PRODUCT)-$(VERSION).ARM.cab

$(PRODUCT)-$(VERSION).exe: setup.nsi $(PRODUCT).ARM.CAB
	$(NSIS) /DLANGUAGE=English /DVERSION=$(VERSION) setup.nsi 

dist:

clean:
	rm -f *.DAT
	rm -f *.CAB
	rm -f *.exe
	rm -f *.zip
