# Buzzby
A pager message receiver app for the custom [Radiolarian](https://github.com/webmariner/Radiolarian) [>(Hexpansion) hardware expansion] to the [EMF](https://www.emfcamp.org/about) [Tildagon badge](https://tildagon.badge.emfcamp.org/).

It consists of:
- a badge app to fetch messages from the Radiolarian when it says there are some waiting
- controller code running on the Radiolarian, which:
  - uses the radio chip to listen for messages
  - temporarily stores received messages
  - tells the badge when there are messages waiting

## Getting the code
The following guide assumes you have a 'Code' folder you keep your git repositories in. Fetch a copy of this repo to that as follows:

    cd Code
    git clone https://github.com/webmariner/Buzzby

## Badge app
The badge client app is a MicroPython program which is stored in the Radiolarian's onboard [M24128](https://www.st.com/resource/en/datasheet/m24128-u.pdf) [>(EEPROM) electrically erasable programmable read-only memory] chip, along with some [information to identify the Hexpansion to the badge OS as being a Radiolarian board](). When the Radiolarian is connected to the badge, the badge automatically loads and runs the app.

### Changing the badge app
Install Python 3 and mpremote. I had Python 3 already so I used `pip install --user mpremote` and made sure `~/.local/bin/` was in my PATH.

Clone the badge-2024-software repo and cd into it:

    git clone https://github.com/emfcamp/badge-2024-software
    cd badge-2024-software

Connect your badge to your machine, make whatever changes you want to Buzzby/src/app.py and then copy it to the Radiolarian via the badge as follows - for example if you've connected your Radiolarian to the slot on the badge's rightmost edge (slot 2):

    mpremote run badge-2024-software/modules/scripts/mount_hexpansions.py + cp Buzzby/src/app.py :/hexpansion_2/

## Radiolarian controller
The code running on the Radiolarian itself is an Arduino Sketch that runs on the onboard Raspberry Pi Pico.

To make changes to, install or monitor the controller code you'll need a lead with a micro-USB plug at one end (to connect to the Raspberry Pi Pico on the Radiolarian) and a USB plug suitable for your PC/laptop (probably USB B) at the other. You'll also need the Arduino IDE, or a text editor of your choice and the Arduino CLI. This guide covers using the CLI.

### Install arduino-cli
For macOS:

    brew install arduino-cli

For Arch linux:

    pacman -S arduino-cli

Instructions for other platforms and more details are available in the [official installation guide](https://docs.arduino.cc/arduino-cli/installation/).


### Compile the sketch
    cd Buzzby
    arduino-cli compile


The following commands assume the pi pico has connected itself in Buzzby mode at /dev/ttyACM0 - if not add:
... `-p /dev/<name of serial device>` or change the port setting in `sketch.yaml`

### Upload changed firmware
    arduino-cli upload

### Connect to Buzzby when running via the USB serial port
    arduino-cli monitor

### Commands while connected to monitor
deb 1 # set debug to level 1
mon # toggle monitoring of receive signal strength etc
