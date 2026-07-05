CC			= gcc
CFLAGS	= -Wall -noixemul -O2
LFLAGS	= -s $(CFLAGS)

rc-ftpd.library: rc-ftpd.library.c
	@echo Making $@...
	@$(CC) -Wall -nostdlib -msmall-code rc-ftpd.library.c -c -o $@.o
#	@$(CC) -Wall -s -O2 -nostdlib -fstrength-reduce -fomit-frame-pointer -msmall-code -mregparm=4 rc-ftpd.library.c -o $@

ftpd: ftpd.c
	@echo Making $@...
	@$(CC) -noixemul -s -O2 -fbaserel -fstrength-reduce ftpd.c -o ftpd


MakeKey: MakeKey.c
	echo Making $@...
	$(CC) $(LFLAGS) -o $@ $<

ftpd_cat.h: ftpd.cd
	@echo Making Catalog code...
	@catcomp DESCRIPTOR ftpd.cd CFILE ftpd_cat.h NOBLOCK NOCODE

ftpd.ct: ftpd.cd
	@echo Making Catalog file...
	@catcomp DESCRIPTOR ftpd.cd CTFILE ftpd.ct

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
	Copy ftpd ftpd_020 ftpd_040 ftpd_060 TO RCFTPd/ QUIET

clean:
	echo Cleaning distribution...
	rm -f rc-ftpd_rev.h ftpd ftpd_020 ftpd_040 ftpd_060 *.o

.SILENT:
