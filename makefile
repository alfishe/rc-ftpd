CC			= gcc
CFLAGS	= -g -Wall -noixemul -O2
LFLAGS	= -s $(CFLAGS)
O_FILES  = ftpd_cat$(EXT).o ftpd$(EXT).o ftpd_cmds$(EXT).o ftpd_mui$(EXT).o \
           ftpd_mui_user$(EXT).o ftpd_mui_path$(EXT).o ftpd_mui_prefs$(EXT).o \
           ftpd_mui_general$(EXT).o ftpd_mui_snoop$(EXT).o ftpd_mui_closed$(EXT).o \
           ftpd_mui_greeting$(EXT).o ftpd_mui_welcome$(EXT).o ftpd_mui_goodbye$(EXT).o \
           ftpd_mui_arexx$(EXT).o asyncio$(EXT).o
# sfv$(EXT).o

ftpd$(EXT): $(O_FILES)
	echo Making $@...
	$(CC) $(LFLAGS) $(PROCESSOR) -o $@ $(O_FILES) -lamiga -lmuimaster  Unix:lib/libnix/swapstack.o

debug: $(O_FILES)
	echo Making $@...
	$(CC) $(CFLAGS) $(PROCESSOR) -o ftpd_debug $(O_FILES) -lamiga -lmuimaster  Unix:lib/libnix/swapstack.o

ftpd$(EXT).o: ftpd.c rc-ftpd_rev.h					;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_cmds$(EXT).o: ftpd_cmds.c						;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui$(EXT).o: ftpd_mui.c							;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_prefs$(EXT).o: ftpd_mui_prefs.c			;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_user$(EXT).o: ftpd_mui_user.c				;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_path$(EXT).o: ftpd_mui_path.c				;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_general$(EXT).o: ftpd_mui_general.c		;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_snoop$(EXT).o: ftpd_mui_snoop.c			;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_closed$(EXT).o: ftpd_mui_closed.c		;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_greeting$(EXT).o: ftpd_mui_greeting.c	;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_welcome$(EXT).o: ftpd_mui_welcome.c		;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_goodbye$(EXT).o: ftpd_mui_goodbye.c		;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_mui_arexx$(EXT).o: ftpd_mui_arexx.c			;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
ftpd_cat$(EXT).o: ftpd_cat.c ftpd_cat.h			;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
asyncio$(EXT).o: asyncio.c								;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<
sfv$(EXT).o: sfv.c										;echo Making $@...;$(CC) $(CFLAGS) $(PROCESSOR) -c -o $@ $<


MakeKey: MakeKey.c
	echo Making $@...
	$(CC) $(LFLAGS) -o $@ $<

translation: ftpd_cat.h
	@echo Making translation...
	FlexCat ftpd.cd ftpd-sprski.ct CATALOG Unix:rc-ftpd/RCFTPd/Catalogs/Srpski/ftpd.catalog

ftpd_cat.h: ftpd.ct ftpd.cd
	@echo Making Catalog code...
	@catcomp DESCRIPTOR ftpd.cd CFILE ftpd_cat.h NOBLOCK NOCODE

ftpd.ct: ftpd.cd
	@echo Making Catalog file...
	@catcomp DESCRIPTOR ftpd.cd CTFILE ftpd.ct
	Copy ftpd.ct ftpd.cd TO RCFTPd/Catalogs/ QUIET

rc-ftpd_rev.h: rc-ftpd_rev.ver
	echo Making version code...
	C:Bump rc-ftpd C QUIET

ver: version
version:			;echo Bumping version...		;Bump rc-ftpd C VER
rev: revision
revision:		;echo Bumping revision...		;Bump rc-ftpd C REV
beta:				;echo Bumping beta version...	;Bump rc-ftpd C BETA

all: ftpd$(EXT)
	$(MAKE) EXT='_020' PROCESSOR='-m68020' --no-print-directory
	$(MAKE) EXT='_040' PROCESSOR='-m68040' --no-print-directory
	$(MAKE) EXT='_060' PROCESSOR='-m68060' --no-print-directory
#	$(MAKE) EXT='_x86' CC='i686be-amithlon-gcc' --no-print-directory
	copy ftpd RCFTPd/ QUIET
	copy ftpd_0?0 _cpu_/RCFTPd/ QUIET
	Delete QUIET FORCE Internet:HTTP/-Software-/rc-ftpd*
	copy RCFTPd/rc-ftpd.readme Internet:HTTP/-Software-/
	copy _cpu_/RCFTPd/rc-ftpd_cpu.readme Internet:HTTP/-Software-/
	copy _loc_/RCFTPd/rc-ftpd_loc.readme Internet:HTTP/-Software-/
	lha -a -e -r -x a Internet:HTTP/-Software-/rc-ftpd.lha RCFTPd RCFTPd.info
	lha -a -e -r -x a Internet:HTTP/-Software-/rc-ftpd_cpu.lha _cpu_/ RCFTPd
	lha -a -e -r -x a Internet:HTTP/-Software-/rc-ftpd_loc.lha _loc_/ RCFTPd

clean:
	echo Cleaning distribution...
	rm -f rc-ftpd_rev.h ftpd ftpd_020 ftpd_040 ftpd_060 *.o

.SILENT:
