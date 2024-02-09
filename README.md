# HD-Door-Opener
Garage door opener control via the bike signal switches


This project used an Arduino Pro Micro (5V - 16MHz) and a MCP2515 Shield Can-Bus (TJA1050) to capture the Bike signals over the CAN bus to activate a garage door remote control.

## Wiring
* MCP2525 is connected to the Arduino SPI
* The remote is powered by teh arduino and pin 7 & 8 trigger two different remote buttons


## Usage
* Activate one of the turn signal (left for garage door 1 or right for door 2)
* Activate High Beam to trigger the remote

Currently installed on my 2018 Fab Bob 114

