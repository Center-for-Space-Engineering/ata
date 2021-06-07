# ata
Testing and verification code for the ATA project at the Center for Space Engineering.

This code is meant to be run on a Raspberry Pi using 5 HATs (2 MCC 118 and 3 MCC 134).

## First time setup
Steps for setup:
1. Download this code
```
git clone https://github.com/nichwall/ata
```
2. Setup submodules (in the `ata` directory which was created)
```
cd ata
git submodule init
git submodule update
```
3. Follow the instructions for setup of the [DAQhats library](https://github.com/mccdaq/daqhats/tree/ec33b8673703958140707beb75b0587be93bd660).
4. Compile program for ATA using `make`

## Collecting Data
To acquire data for the ATA vaccuum chamber test, two scripts are required.

First, run `make` to ensure `./bin/ATA_DAQ` is compiled. This script collects data from the thermocouples and voltage readers. It also converts the RPM and Pressure and stores them in a log file.
* Voltages and pressure at 1 Hz - `./logs/voltages.csv`
* Voltages and pressure at 1/60 Hz - `./logs/voltages_slow.csv`
* Thermocouples at 1 Hz - `./logs/thermo.csv`
* Thermocouples at 1/60 Hz - `./logs/thermo_slow.csv`
* RPM raw data - `./logs/rpm.csv`

The second script is run using the command `python3 bus_pirate.py` to collect data from the RTDs. Data is logged to `./logs/SPI_logging.csv`.

## Viewing Data
Run the command `python live_plot.py` to view the live data in a graph. The program will automatically terminate after about two hours and can be run again to continue viewing the state of the test.
