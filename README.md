Black Magic Probe
=================

[![Build Status](https://travis-ci.org/blacksphere/blackmagic.svg?branch=master)](https://travis-ci.org/blacksphere/blackmagic)
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/blacksphere/blackmagic?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Donate](https://img.shields.io/badge/paypal-donate-blue.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=N84QYNAM2JJQG)
[![Kickstarter](https://img.shields.io/badge/kickstarter-back%20us-14e16e.svg)](https://www.kickstarter.com/projects/esden/1bitsy-and-black-magic-probe-demystifying-arm-prog)

Firmware for the Black Magic Debug Probe.

The Black Magic Probe is a modern, in-application debugging tool for
embedded microprocessors. It allows you see what is going on 'inside' an
application running on an embedded microprocessor while it executes. It is
able to control and examine the state of the target microprocessor using a
JTAG or Serial Wire Debugging (SWD) port and on-chip debug logic provided
by the microprocessor. The probe connects to a host computer using a
standard USB interface. The user is able to control exactly what happens
using the GNU source level debugging software, GDB.

See online documentation at https://github.com/blacksphere/blackmagic/wiki

Binaries from the latest automated build are at http://builds.blacksphere.co.nz/blackmagic

Staging branch
=================
This branch is for including pull requests and patches the author thinks are
sensible. Users are invited to test and report success/failure on the Gitter
channel to help mantainers to decide to integrate in the master branch or not.

Included branches:

mingw_ftdi
=========
ENHANCEMENT: Allow to compile libftdi platform also mingw and cygwin.
ENHANCEMENT: Allow "make all_platforms" to succeed on system using
             recent libftdi1.
probe_rework
============
FIX:         Add again additional SWD cycles on detach.
             Otherwise many targets do not restart.
             Introduced with #391, 2018 Sep 7
CONSISTANCY: Restore old connect_srst behaviour.
             #357 introduced the changed syntax 2018 June 21
ENHANCEMENT: Make probe much less intrusive.
             Hard to probe STM32F7/H7 provide ADI DPv2 where CPU ID can be read
             direct from the DP
ENHANCEMENT: On STM32F7/H7, where DBGMCU must be modified for
             debugging, restore DBGMCU on detach.
ENHANCEMENT: With connect under reset, only release SRST when Halt on reset
             has been set up.
             Should help when user program disables debug access and with
             attaching to devices in deep sleep state.
