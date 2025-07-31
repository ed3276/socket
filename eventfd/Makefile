OUTDIR := build
CFLAGS := -g -std=c++14 -pthread

all: $(OUTDIR)/eventfd $(OUTDIR)/pipe2

$(OUTDIR)/eventfd: $(OUTDIR)/eventfd.o
	g++ $(CFLAGS) $^ -o $@

$(OUTDIR)/eventfd.o: eventfd.cpp
	g++ $(CFLAGS) -c $^ -o $@

$(OUTDIR)/pipe2: $(OUTDIR)/pipe2.o
	g++ $(CFLAGS) $^ -o $@

$(OUTDIR)/pipe2.o: pipe2.cpp
	g++ $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf $(OUTDIR)/*
