all: bin/gsm bin/skymap share/gsm/components.dat share/gsm/component_maps_408locked.dat

bin/gsm: src/gsm.f
	mkdir -p bin
	gfortran -w -ffixed-line-length-132 -std=legacy -o $@ $^

bin/skymap: src/skymap.c
	mkdir -p bin
	gcc $^ -o $@ -Iinclude -O3 -Llib -Wl,-rpath,'$$ORIGIN/../lib' -lchealpix -lm

share/gsm/%.dat:
	mkdir -p share/gsm
	wget -P share/gsm -c https://github.com/TREND50/skymap/releases/download/gsm-data/$*.dat

.PHONY: clean
clean:
	rm -rf bin
