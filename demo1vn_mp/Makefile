OUTDIR := build
CFLAGS := -g -std=c++14 -pthread

all: $(OUTDIR)/client $(OUTDIR)/server

$(OUTDIR)/client: $(OUTDIR)/client.o
	g++ $(CFLAGS) $^ -o $@

$(OUTDIR)/client.o: client.cpp
	g++ $(CFLAGS) -c $^ -o $@

$(OUTDIR)/server: $(OUTDIR)/server.o
	g++ $(CFLAGS) $^ -o $@

$(OUTDIR)/server.o: server.cpp
	g++ $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf $(OUTDIR)/*
