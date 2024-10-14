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

HP_MAIN_SRC=$(EXAMPLESDIR)/hp_main.c $(SRCDIR)/record.c $(SRCDIR)/hp_file.c
BF_MAIN_SRC=$(EXAMPLESDIR)/bf_main.c $(SRCDIR)/record.c
HT_MAIN_SRC=$(EXAMPLESDIR)/ht_main.c $(SRCDIR)/record.c $(SRCDIR)/ht_table.c

HP_MAIN=$(BUILDDIR)/hp_main
BF_MAIN=$(BUILDDIR)/bf_main
HT_MAIN=$(BUILDDIR)/ht_main

all: hp bf ht

hp:
	@echo "Compiling hp_main ..."
	$(CC) $(CFLAGS) $(LDFLAGS) $(HP_MAIN_SRC) $(LIBS) -o $(HP_MAIN) $(OPTIMIZE) -fsanitize=leak 

bf:
	@echo "Compiling bf_main ..."
	$(CC) $(CFLAGS) $(LDFLAGS) $(BF_MAIN_SRC) $(LIBS) -o $(BF_MAIN) $(OPTIMIZE) -w

ht:
	@echo "Compiling ht_main ..."
	$(CC) $(CFLAGS) $(LDFLAGS) $(HT_MAIN_SRC) $(LIBS) -o $(HT_MAIN) $(OPTIMIZE) -w

clean:
	rm -f $(HP_MAIN) $(BF_MAIN) $(HT_MAIN) data.db block_example.db

.PHONY: all hp bf ht clean
