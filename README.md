# HOS - Hacker Operating System

**Version**: Alpha 0.0.1  
**Author**: Brian Riff  
**Release Date**: April 12, 2025

---

HOS is a lightweight Linux-based operating system framework built in C.
It is made for hackers, and includes low-level networking tools, hashing, and more! Currently in early alpha, HOS is modular and built for extensibility.

---

## Features
- **Ping Module**: Basic ICMP-based connectivity checks.
- **Modular Architecture**: Easy to extend with your own tools and libraries.
- **Custom Shell**: Built-in command parsing for embedded usage and command execution.
- **NetWatch**: Real-time network interface monitor with live packet stats and top host tracking.

Netwatch runnning on my machine:

![image](https://github.com/user-attachments/assets/90d20eab-249b-4b66-8633-eb4a434d5fe5)

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

## Running the OS
- Compiling the OS:
  source compile.sh
- Running the OS:
  source compile.sh run
