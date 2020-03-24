all: bin/gsm bin/skymap

bin/gsm: src/gsm.f
	mkdir -p bin
	gfortran -w -ffixed-line-length-132 -std=legacy -o $@ $^

bin/skymap: src/skymap.c
	mkdir -p bin
	gcc $^ -o $@ -Iinclude -O3 -Llib -Wl,-rpath,'$$ORIGIN/../lib' -lchealpix -lm

.PHONY: clean
clean:
	rm -rf bin
