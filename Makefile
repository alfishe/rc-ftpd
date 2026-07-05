# rc-ftpd cross-compilation Makefile
# Requires bebbo's amiga-gcc toolchain

CC = m68k-amigaos-gcc
VERSION = 2.60

# Include paths for dependencies
MUI_INC = libs/mui38dev/MUI/Developer/C/Include
NLIST_INC = libs/MCC_NList/Developer/C/include
TEXTINPUT_INC = libs/MCC_Textinput/Developer/C/Include
LAMP_INC = libs/MCC_Lamp/Developer/C/Include
BBALANCE_INC = libs/MCC_BetterBalance/MCC_BetterBalance/Developer/C/Include

COMPAT_INC = libs/compat

INCLUDES = -I$(COMPAT_INC) -I$(MUI_INC) -I$(NLIST_INC) -I$(TEXTINPUT_INC) -I$(LAMP_INC) -I$(BBALANCE_INC)

# Suppress legacy code warnings that would require major refactoring
# Suppress legacy code warnings that would require major refactoring
NOWARN = -Wno-pointer-sign -Wno-attributes -Wno-int-conversion -Wno-missing-braces \
         -Wno-incompatible-pointer-types -Wno-unused-but-set-variable \
         -Wno-format -Wno-uninitialized

CFLAGS = -Wall $(NOWARN) -noixemul -Os -fomit-frame-pointer $(INCLUDES)
LFLAGS = -s $(CFLAGS)

SRCDIR = src
BUILDDIR = build
TARGET = $(BUILDDIR)/ftpd

SRCS = ftpd_cat.c ftpd.c ftpd_cmds.c ftpd_mui.c \
       ftpd_mui_user.c ftpd_mui_path.c ftpd_mui_prefs.c \
       ftpd_mui_general.c ftpd_mui_snoop.c ftpd_mui_closed.c \
       ftpd_mui_greeting.c ftpd_mui_welcome.c ftpd_mui_goodbye.c \
       ftpd_mui_arexx.c asyncio.c

OBJS = $(SRCS:%.c=$(BUILDDIR)/%.o)

.PHONY: all clean dist release

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^ -lamiga -lmui -lgcc

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

# Copy binary to distribution folder
dist: $(TARGET)
	cp $(TARGET) dist/RCFTPd/ftpd
	@echo "Binary copied to dist/RCFTPd/ftpd"

# Create Aminet release archive
release: dist
	@mkdir -p release
	cd dist && lha -c ../release/rc-ftpd.lha RCFTPd RCFTPd.info
	cp dist/RCFTPd/rc-ftpd.readme release/rc-ftpd.readme
	@echo ""
	@echo "Release created:"
	@ls -la release/rc-ftpd.lha release/rc-ftpd.readme
