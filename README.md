# ata
Testing and verification code for the ATA project at the Center for Space Engineering.

## Collecting Data
To acquire data for the ATA vaccuum chamber test, two scripts are required.

First, run `make` to ensure `./bin/ATA_DAQ` is compiled. This script collects data from the thermocouples and voltage readers. It also converts the RPM and Pressure and stores them in a log file.

The second script is run using the command `python3 bus_pirate.py` to collect data from the RTDs.

## Viewing Data
Run the command `python live_plot.py` to view the live data in a graph. The program will automatically terminate after about two hours and can be run again to continue viewing the state of the test.
