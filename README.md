# ace
ACE is a minimal, fast, and VIM-inspired text editor for the terminal, written in modern C++. It features custom memory-efficient buffer management, multiple editing modes, and real-time rendering — all under 2000 lines of code.

## Platform Support

Currently supports **UNIX-like systems only** (Linux/macOS).

- Uses `termios`, `unistd.h`, and `ioctl`
- Tested in terminal emulators like iTerm2, GNOME Terminal, and VSCode's terminal

For Windows support, use WSL or Git Bash.
