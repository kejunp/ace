#pragma once

#include <termios.h>

// Initializes raw mode for terminal input (non-canonical, no echo, etc.)
void enable_raw_mode();

// Restores the original terminal mode (on exit)
void disable_raw_mode();

// Clears the entire screen and resets cursor to top-left
void term_clear_screen();

// Moves the terminal cursor to (row, col), 1-based
void move_cursor(int row, int col);