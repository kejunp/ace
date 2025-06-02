#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <csignal>
#include <cstdlib>
#include "buffer.h"
#include "command.h"
#include <vector>
#include <string>

termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int get_window_size(int& rows, int& cols) {
    winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) return -1;
    rows = ws.ws_row;
    cols = ws.ws_col;
    return 0;
}

void clear_screen() {
    std::cout << "\x1b[2J\x1b[H";
}

void move_cursor(int row, int col) {
    std::cout << "\x1b[" << row << ";" << col << "H";
}

enum class EditorMode { Normal, Insert, Command };
EditorMode mode = EditorMode::Normal;
std::string command_buffer;
std::vector<std::string> command_history;
int history_index = 0;

void render_buffer(EditorBuffer &buf, int term_rows, int term_cols) {
    clear_screen();

    int row = 1, col = 1;
    EditorBuffer::Node* sentinel = buf.get_head();
    EditorBuffer::Node* cursor = buf.get_cursor();
    EditorBuffer::Node* p = sentinel->next;

    while (p != sentinel && row <= term_rows - 1) {
        move_cursor(row, col);
        std::cout << (p->data == '\n' ? ' ' : p->data);

        // Advance cursor position
        if (p->data == '\n') {
            row++;
            col = 1;
        } else {
            col++;
            if (col > term_cols) {
                row++;
                col = 1;
            }
        }

        // If this node is the cursor, draw block after the char
        if (p == cursor && row <= term_rows - 1) {
            move_cursor(row, col);
            std::cout << "\x1b[7m \x1b[0m";  // block cursor
        }

        p = p->next;
    }

    // Special case: cursor is at sentinel (end of buffer)
    if (cursor == sentinel && row <= term_rows - 1) {
        move_cursor(row, col);
        std::cout << "\x1b[7m \x1b[0m";
    }

    // Status bar
    move_cursor(term_rows, 1);
    if (mode == EditorMode::Command) {
        std::cout << ":" << command_buffer << " ";
    } else {
        std::cout << "\x1b[44m-- ACE (" << (mode == EditorMode::Insert ? "INSERT" : "NORMAL") << ") --\x1b[0m";
    }

    std::cout.flush();
}

int main(int argc, char** argv) {
    /**
     * TODO:
     * add ./ace filename later
     */
    (void) argc;
    (void) argv;
    enable_raw_mode();

    EditorBuffer buffer;
    int rows, cols;
    get_window_size(rows, cols);

    while (true) {
        render_buffer(buffer, rows, cols);

        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;

        if (mode == EditorMode::Command) {
            if (c == '\r') {
                CommandResult result = run_command(buffer, command_buffer);
                if (result == CommandResult::QUIT) break;
                mode = EditorMode::Normal;
                command_history.push_back(command_buffer);
                history_index = command_history.size();
                command_buffer.clear();
            } else if (c == 127 || c == 8) {
                if (!command_buffer.empty()) command_buffer.pop_back();
            } else if (c == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, seq, 2) == 2) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'A' && history_index > 0) {
                            history_index--;
                            command_buffer = command_history[history_index];
                        } else if (seq[1] == 'B' && history_index < (int)command_history.size() - 1) {
                            history_index++;
                            command_buffer = command_history[history_index];
                        }
                    }
                }
            } else {
                command_buffer += c;
            }

        } else if (mode == EditorMode::Insert) {
            if (c == 27) {
                mode = EditorMode::Normal;
            } else if (c == 127 || c == 8) {
                buffer.backspace_char();
            } else if (c == '\r') {
                buffer.insert('\n');
            } else if (isprint(c)) {
                buffer.insert(c);
            }

        } else {
            // Normal Mode
            if (c == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, seq, 2) == 2) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'C') buffer.move_right();
                        else if (seq[1] == 'D') buffer.move_left();
                        else if (seq[1] == 'A') buffer.move_up();
                        else if (seq[1] == 'B') buffer.move_down();
                    }
                }
            } else {
                switch (c) {
                    case ':':
                        command_buffer.clear();
                        mode = EditorMode::Command;
                        break;
                    case 'i':
                        mode = EditorMode::Insert;
                        break;
                    case 'h':
                        buffer.move_left();
                        break;
                    case 'l':
                        buffer.move_right();
                        break;
                    case 's':
                        buffer.move_start();
                        break;
                    case 'e':
                        buffer.move_end();
                        break;
                    case 'j':
                        buffer.move_down();
                        break;
                    case 'k':
                        buffer.move_up();
                        break;
                    case 127:
                    case 8:
                        buffer.backspace_char();
                        break;
                    default:
                        break;
                }
            }
        }
    }

    clear_screen();
    return 0;
}