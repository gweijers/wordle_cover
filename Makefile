TARGET=wordle_cover
SOURCES=wordle_cover.c dlx.h dlx.c Makefile
EXTRA=
PREFIX?=$(HOME)

CC=gcc
CFLAGS=-O3 -Wall -std=gnu99 -mtune=native -flto=full
LIBS=-lm
#CFLAGS=-O3 -Wall -std=gnu99 -mtune=native
#CFLAGS=-g -Wall -std=gnu99
#CFLAGS=-pg -O -Wall -std=gnu99 -DNDEBUG

CFLAGS+= -Wextra
CFLAGS+= -Werror -Wfloat-equal -Wconversion -Wparentheses -pedantic -Wunused-parameter -Wunused-variable -Wreturn-type \
		 -Wunused-function -Wredundant-decls -Wunused-value -Wswitch-default -Wuninitialized -Winit-self
OS=$(shell uname -s | tr A-Z a-z)
ifeq ($(OS),linux)
LIBS+=-lrt
endif

ODIR=.obj
CFILES=$(filter %.c,$(SOURCES))
HFILES=$(filter %.h,$(SOURCES))
OFILES=$(patsubst %.c,$(ODIR)/%.o,$(CFILES))
DFILES=$(patsubst %.c,$(ODIR)/%.d,$(CFILES))
SFILES=$(patsubst %.c,%.s,$(CFILES))

CONTENT=$(SOURCES) $(EXTRA)
STAGEDIR=sudoku-1.0
TARBALL=$(STAGEDIR).tar.bz2
COPIES=$(patsubst %,$(STAGEDIR)/%,$(CONTENT))
CPDIRS=$(sort $(patsubst %/,%,$(dir $(COPIES))))

JUNK=$(CONTENT:=~) $(ODIR) $(TARGET) $(STAGEDIR) $(SFILES) $(TARBALL) gmon.out build dist

$(TARGET):      $(OFILES)
	$(CC) $(CFLAGS) $(OFILES) $(LIBS) -o $@

$(OFILES):      $(ODIR)/%.o:    %.c
	$(CC) $(CFLAGS) -c -MMD $< -o $@

$(OFILES):      Makefile | $(ODIR)

$(SFILES):              %.s:    %.c
	$(CC) $(CFLAGS) -S $<

$(SFILES):              ALWAYS

ALWAYS:

$(ODIR):
	mkdir -p $@

clean:
	rm -rf $(JUNK)

retab:
	wsfix -g $(CFILES) $(HFILES)
	wsfix -2 Makefile

src:    $(TARBALL)


$(TARBALL):     $(COPIES)
	tar cf - $(STAGEDIR) | bzip2 -9 > $@
	-rm -rf $(STAGEDIR)

$(COPIES):      $(STAGEDIR)/%:  %
	cp $< $@; chmod 644 $@

$(COPIES):      | $(CPDIRS)

$(CPDIRS):      %:
	mkdir -p -m 755 $@

install:        $(PREFIX)/bin/$(TARGET)

$(PREFIX)/bin/$(TARGET):        $(TARGET) | $(PREFIX)/bin
	install $< $@
	chmod 755 $@


$(PREFIX)/bin:
	mkdir -p $@

.PHONY: clean retab src ALWAYS

-include $(DFILES)

# vim: set ts=4 sts=4 sw=4 noet:
