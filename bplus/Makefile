CC=gcc
CFLAGS=-g3 -I./include 
LDFLAGS=-L./lib -Wl,-rpath=./lib
LIBS=-lbf
OPTIMIZE=-O2

SRCDIR=./src
INCDIR=./include
LIBDIR=./lib
EXAMPLESDIR=./examples
BUILDDIR=./build

BP_MAIN_SRC=$(EXAMPLESDIR)/bp_main.c $(SRCDIR)/record.c $(SRCDIR)/bp_file.c $(SRCDIR)/bp_datanode.c $(SRCDIR)/bp_indexnode.c
BP_MAIN=$(BUILDDIR)/bplus_main

all: bp

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

bp: $(BUILDDIR)
	@echo "Compiling bplus_main ..."
	$(CC) $(CFLAGS) $(LDFLAGS) $(BP_MAIN_SRC) $(LIBS) -o $(BP_MAIN) $(OPTIMIZE) -w

clean:
	rm -f $(BP_MAIN) data.db
	@rm -rf $(BUILDDIR)

.PHONY: all bp clean
