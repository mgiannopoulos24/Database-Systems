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
DBREAD_SRC=$(EXAMPLESDIR)/db_read.c $(SRCDIR)/record.c $(SRCDIR)/ht_table.c

HP_MAIN=$(BUILDDIR)/hp_main
BF_MAIN=$(BUILDDIR)/bf_main
HT_MAIN=$(BUILDDIR)/ht_main
DBREAD=$(BUILDDIR)/db_check

all: hp bf ht dbcheck

hp:
	@echo "Compiling hp_main ..."
	$(CC) $(CFLAGS) $(LDFLAGS) $(HP_MAIN_SRC) $(LIBS) -o $(HP_MAIN) $(OPTIMIZE) -w 

bf:
	@echo "Compiling bf_main ..."
	$(CC) $(CFLAGS) $(LDFLAGS) $(BF_MAIN_SRC) $(LIBS) -o $(BF_MAIN) $(OPTIMIZE) -w

ht:
	@echo "Compiling ht_main ..."
	$(CC) $(CFLAGS) $(LDFLAGS) $(HT_MAIN_SRC) $(LIBS) -o $(HT_MAIN) $(OPTIMIZE) -w

dbcheck:
	@echo "Compiling db_read ..."
	$(CC) $(CFLAGS) $(LDFLAGS) $(DBREAD_SRC) $(LIBS) -o $(DBREAD) $(OPTIMIZE) -w

clean:
	rm -f $(HP_MAIN) $(BF_MAIN) $(HT_MAIN) $(DBREAD) data.db block_example.db

.PHONY: all hp bf ht dbcheck clean