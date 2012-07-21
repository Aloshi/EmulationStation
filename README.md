EmulationStation
================

A simple front-end for emulators made with SDL, designed for controller navigation. Developed for use with the Raspberry Pi and RetroArch.


Building
========

EmulationStation has a few dependencies. For building, you'll probably need `libsdl1.2-dev`, `libsdl-ttf2.0-dev`, and `libboost-filesystem-dev`.

You can build it by just running `make`.

Configuring
===========

At the moment, the software looks for a file named "systems.cfg" in the directory it is run from. An example is included.

