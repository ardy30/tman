#
# tMan Config
#

OUTDIR=build
BIN=tmancfg.exe

VPATH=../share
SRC=$(wildcard *.cpp)\
	$(wildcard ../share/*.cpp)
	
RC=tManCfg.rc
LIB=

# -----------------------------------------------------------------------------

TMP_OBJ=$(addprefix $(OUTDIR)/, $(patsubst %.cpp,%.obj,$(notdir $(SRC))))
RES=$(addprefix $(OUTDIR)/, $(RC:.rc=.res))
OBJ=$(TMP_OBJ) $(RES)


# 300 for eVC 3.0
CEVersion=420
CePlatform=WIN32_PLATFORM_PSPC=400
CESubsystem=windowsce,4.20

CECrt=C
CECrtDebug=C
CECrtMT=C
CECrtMTDebug=C
CENoDefaultLib=libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib
CEx86Corelibc=corelibc.lib

RSC=/opt/wince-arm/bin/rc
CPP=/opt/wince-arm/bin/clarm
LINK=/opt/wince-arm/bin/link

# minimal include
INCLUDE=\
	/I z:\\opt\\wince-arm\\wce400\\include\\armv4 \
	/I z:\\opt\\wince-arm\\wce400\\mfc\\Include

LIB_PATH=\
	/LIBPATH:z:\\opt\\wince-arm\\wce400\\lib\\Armv4 \
	/LIBPATH:z:\\opt\\wince-arm\\wce400\\mfc\\Lib\\armv4

RSC_INCLUDE=\
	/i z:\\opt\\wince-arm\\wce400\\include\\armv4 \
	/i z:\\opt\\wince-arm\\wce400\\mfc\\Include


# debug
#RSC_FLAGS=/l 0x409 /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "DEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CePlatform)" /d "ARM" /d "_ARM_" /d "ARMV4" /d "_AFXDLL" /d "AFX_TARG_$(LANG)" /r 
#CPP_FLAGS=/nologo /W3 /Zi /Od /D "DEBUG" /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CePlatform)" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /M$(CECrtMTDebug) /c 
#LINK32_FLAGS=/nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"wWinMainCRTStartup" /incremental:yes /debug /subsystem:$(CESubsystem) /align:"4096" /MACHINE:ARM 

# release
RSC_FLAGS=/l 0x409 /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "NDEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CePlatform)" /d "ARM" /d "_ARM_" /d "ARMV4" /d "_AFXDLL" /r 
CPP_FLAGS=/nologo /W3 /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CePlatform)" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_AFXDLL" /D "TMAN_CONFIG" /D "LOGGING" /O2 /M$(CECrtMT) /c 
LINK32_FLAGS=/nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"wWinMainCRTStartup" /subsystem:$(CESubsystem) /align:"4096" /MACHINE:ARM 

## --- ##

all: $(OUTDIR)/$(BIN)

$(OUTDIR):
	@test -d "$(OUTDIR)" || mkdir "$(OUTDIR)"

# executable
$(OUTDIR)/$(BIN): $(OUTDIR) $(OBJ)
	@echo "LINK  $@"
	@$(LINK) $(LINK32_FLAGS) $(LIB_PATH) /OUT:$@ $(OBJ) $(LIB)

# main files
$(OUTDIR)/%.obj: %.cpp
	@$(CPP) $(CPP_FLAGS) $(INCLUDE) /Fo$@ $<

$(OUTDIR)/%.res: %.rc
	@echo "RC    $@"
	@$(RSC) $(RSC_FLAGS) $(RSC_INCLUDE) /fo$@ $<

dep:
	@$(CC) -MM -DDEPEND -D__AFXWIN_H__ -D_WIN32_WCE=$(CEVersion) -DARM -D_ARM_ -DARMV4 -DUNDER_CE=$(CEVersion) -D$(CePlatform) -DUNICODE -D_UNICODE -DLOGGING -I/opt/wince-arm/wce400/include/armv4 $(SRC) | sed 's/^\([^ .]*\)\.o/$(OUTDIR)\/\1.obj/' > .dep

clean:
	rm -f $(OUTDIR)/*.obj
	rm -f $(OUTDIR)/*.res
	rm -f $(OUTDIR)/$(BIN)

-include .dep
