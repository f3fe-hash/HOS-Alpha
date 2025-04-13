# HOS - Hacker Operating System

**Version**: Alpha 0.0.1  
**Author**: Brian Riff  
**Release Date**: April 10, 2025

---

HOS is a lightweight Linux-based operating system framework for low-level networking tools, systems experimentation, and custom development environments. Currently in early alpha, HOS is modular and built for extensibility.

---

## Features

- **NetWatch**: Real-time network interface monitor with live packet stats and top host tracking.
- **Ping Module**: Basic ICMP-based connectivity checks.
- **Modular Architecture**: Easy to extend with your own tools and libraries.
- **Custom Shell**: Built-in command parsing for embedded usage and command execution.

---

## Commands
- **help**:
  Print a help message containing all of the commands, and how to use them.
- **exit**:
  Exit the OS, Ctrl-C also works.
- **version**:
  Print current OS Version.
- **echo**:
  Echo command arguments to the screen.
- **clear**:
  Clear the screen
- **setport**:
  Set the OS's port (Can also be achived at startup by specifying -p <port> when running)
- **ping**:
  Ping an host
- **netwatch**:
  Watch a network interface
