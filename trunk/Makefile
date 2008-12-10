#
# tMan
#

product=tMan
version=0.9.11

#CABWIZ=wine z:\\opt\\wince-arm\\wce400\\tools\\cabwiz.exe
#CABWIZ=wine /home/daprofik/.wine/drive_c/Program\ Files/Windows\ CE\ Tools/wce420/POCKET\ PC\ 2003/Tools/Cabwiz.exe
CABWIZ=wine c:\\Program\ Files\\Windows\ CE\ Tools\\wce420\\POCKET\ PC\ 2003\\Tools\\Cabwiz.exe

all:
	$(MAKE) -C tman
	$(MAKE) -C tmancfg
	$(MAKE) -C setup

dep:
	$(MAKE) -C tman dep
	$(MAKE) -C tmancfg dep
	$(MAKE) -C setup dep

clean:
	$(MAKE) -C tman clean
	$(MAKE) -C tmancfg clean
	$(MAKE) -C setup clean

#
# Make distribution package
#
dist:
	@echo "Making distribution package..."
	@$(CABWIZ) $(product).inf /CPU ARM
	@mv -f $(product).ARM.CAB $(product)-$(version).cab
	@rm $(product).ARM.DAT

