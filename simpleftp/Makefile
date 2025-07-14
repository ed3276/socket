OUTDIR := build
CFLAGS := -g -std=c++11 -pthread

TARGET := $(OUTDIR)/client $(OUTDIR)/server
all: $(TARGET)

$(OUTDIR)/client: $(OUTDIR)/client.o
	g++ $(CFLAGS) $^ -o $@

$(OUTDIR)/client.o: client.cpp
	g++ $(CFLAGS) -c $< -o $@

$(OUTDIR)/server: $(OUTDIR)/server.o
	g++ $(CFLAGS) $^ -o $@

$(OUTDIR)/server.o: server.cpp threadpool.hpp
	g++ $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET)
	rm -rf $(OUTDIR)/*.o
