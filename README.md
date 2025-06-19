# Donation Tracker

Donation Tracker is a Qt-based C++ application for managing donations, including donor information, donation records, and generating donation letters. It uses SQLite for data storage and provides a graphical user interface.

## Features
- Add, edit, and delete donors and donations
- Set and display organization details
- Generate donation letters for a specified year
- Search donors by various fields
- Input validation for names, addresses, emails, etc.

## License
This program is free software licensed under the GNU General Public License version 3 (GPLv3). See the [COPYING](COPYING) file for details.

## Requirements
- Qt 5.15.x (qtbase5-dev, qtbase5-dev-tools)
- SQLite3 (libsqlite3-dev)
- C++ compiler (g++)
- Linux (tested on Linux Mint 22.1/Ubuntu 22.04)

## Installation
1. Install dependencies:
   ```bash
   sudo apt update
   sudo apt install qtbase5-dev qtbase5-dev-tools libsqlite3-dev g++ make
