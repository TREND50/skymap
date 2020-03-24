# Diffuse sky radio noise with GSM

2. Clone this repo & compile `gsm` + `skymap` with the provided
   [Makefile](Makefile) as:
   ```bash
   git clone https://github.com/TREND50/skymap
   cd skymap
   make
   ```

2. Tabulate the radio signal as frequency using GSM. See
   [run-gsm.py](share/scripts/run-gsm.py).

3. Compute the corresponding antenna noise using the provided effectie area and
   `skymap`. See e.g. [compute-skymap.py](share/scripts/compute-skymap.py).

> The results can be checked with the [show-gsm.py](share/scripts/show-gsm.py)
> and [show-skymap.py](share/scripts/show-skymap.py) scripts.
