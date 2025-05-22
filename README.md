# HOS – Hacker Operating System  
⚡ *A modular, C-based Linux framework for hackers and tinkerers.*

**Version**: Alpha 1.2
**Author**: Brian Riff  
**Release Date**: May 21, 2025  

---

HOS is a lightweight operating system framework built in C, designed for hackers, developers, and system-level explorers. It includes real-time networking tools, a custom shell, and modular architecture — all open and extensible.

🟥 Warning: This OS does need superuser privileges to run, and uses them for creating files and directories, along with accessing core networking utilities.

Whether you're learning OS internals or building your own low-level tools, HOS provides a clean, powerful foundation.

👉 **[Visit the Website](https://f3fe-hash.github.io/HOS-Alpha-Website/)**  
⭐️ Star the repo if you find it interesting!

---

## 🚀 Features

- **🛠 Modular Architecture**: Easily add your own tools and system modules.
- **🌐 NetWatch**: Real-time interface monitoring, live packet stats, and top-host tracking.
- **📡 Ping Utility**: Basic ICMP-based connectivity checks.
- **💻 Custom Shell**: Built-in command parsing and execution.

---

## NEW!
- **Full HPM!**: Full HOS Package Manager (HPM), download, install, and purge packages!

## 🖼 NetWatch in Action

Here’s what NetWatch looks like while monitoring the `wlo1` (main Wi-Fi) interface:

![NetWatch Screenshot](https://github.com/user-attachments/assets/4ed863ca-6eee-4682-a9e3-2cc66ac50a19)

---

## 🧾 Available Commands

| Command   | Description |
|-----------|-------------|
| `help`    | Print help message with available commands. |
| `exit`    | Exit the OS. (Ctrl+C also works.) |
| `version` | Display current OS version. |
| `echo`    | Echo input text to the screen. |
| `clear`   | Clear the terminal screen. |
| `setport` | Set OS communication port. (`-p <port>` also works at launch.) |
| `ping`    | Ping a host using ICMP. |
| `netwatch`| Launch network monitoring interface. |
| `gui`     | Open Desktop GUI |
| `hpm`     | HOS Package Manger |
| `run`     | Run an installed binary |

---

## ⚙️ Getting Started

```bash
# Clone the repository
git clone https://github.com/f3fe-hash/HOS-Alpha.git
cd HOS-Alpha

# Compile the OS
source compile.sh

# Run the OS
source run.sh
