#include "term.h"
#include <unistd.h>
#include <termios.h>
#include <cstdlib>
#include <iostream>
#include <sys/ioctl.h>

static termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        std::perror("tcgetattr");
        std::exit(1);
    }
    atexit(disable_raw_mode);

    termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);  // disable echo, canonical mode, Ctrl-C/Z
    raw.c_iflag &= ~(IXON | ICRNL);          // disable Ctrl-S/Q and CR-to-NL
    raw.c_oflag &= ~(OPOST);                 // disable post-processing (e.g., newline -> CRLF)

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        std::perror("tcsetattr");
        std::exit(1);
    }
}

void term_clear_screen() {
    std::cout << "\x1b[2J\x1b[H";
}

void move_cursor(int row, int col) {
    std::cout << "\x1b[" << row << ";" << col << "H";
}
