#include <iostream>
#include <fstream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <vector>
#include "buffer.h"

termios orig_termios;

enum class EditorMode {
    Normal,
    Insert,
    Command
};

EditorMode mode = EditorMode::Normal;
std::string command_buffer;
std::vector<std::string> command_history;
int history_index = 0;

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

int get_window_size(int &rows, int &cols) {
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

bool execute_command(EditorBuffer &buffer) {
    if (command_buffer.starts_with("s/")) {
        auto parts = command_buffer.substr(2);
        size_t delim = parts.find('/');
        if (delim != std::string::npos) {
            std::string pattern = parts.substr(0, delim);
            std::string replacement = parts.substr(delim + 1);
            buffer.substitute_all(pattern, replacement);
        }
    } else if (command_buffer == "q" || command_buffer == "quit") {
        return true;
    }
    command_history.push_back(command_buffer);
    history_index = command_history.size();
    return false;
}

void render_buffer(EditorBuffer &buf, int term_rows, int term_cols) {
    clear_screen();

    int row = 1, col = 1;
    int cursor_row = -1, cursor_col = -1;

    EditorBuffer::Node* sentinel = buf.get_head();
    EditorBuffer::Node* cursor = buf.get_cursor();
    EditorBuffer::Node* p = sentinel->next;

    while (p != sentinel && row <= term_rows - 1) {
        // Save position where visual cursor should appear (after `cursor`)
        if (p == cursor->next) {
            cursor_row = row;
            cursor_col = col;
        }

        move_cursor(row, col);
        std::cout << (p->data == '\n' ? ' ' : p->data);

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

        p = p->next;
    }

    // If cursor is at end (cursor->next == sentinel), place caret after last char
    if (cursor->next == sentinel && row <= term_rows - 1) {
        cursor_row = row;
        cursor_col = col;
    }

    // Render the block cursor
    if (cursor_row != -1 && cursor_col != -1) {
        move_cursor(cursor_row, cursor_col);
        std::cout << "\x1b[7m \x1b[0m";
    }

    // Draw command/status bar
    move_cursor(term_rows, 1);
    if (mode == EditorMode::Command) {
        std::cout << ":" << command_buffer << " ";
    } else {
        std::cout << "\x1b[44m-- ACE (" << (mode == EditorMode::Insert ? "INSERT" : "NORMAL") << ") --\x1b[0m";
    }

    // Restore terminal cursor visibility and position
    move_cursor(cursor_row != -1 ? cursor_row : term_rows, cursor_col != -1 ? cursor_col : 1);
    std::cout << "\x1b[?25h";
    std::cout.flush();
}


int main(int argc, char** argv) {
    enable_raw_mode();

    EditorBuffer buffer;
    int rows, cols;
    get_window_size(rows, cols);

    if (argc > 1) {
        std::ifstream file(argv[1]);
        if (file) {
            char ch;
            while (file.get(ch)) {
                buffer.insert(ch);
            }
        } else {
            std::cerr << "Failed to open file: " << argv[1] << "\n";
            return 1;
        }
    }

    while (true) {
        render_buffer(buffer, rows, cols);

        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;

        if (mode == EditorMode::Command) {
            if (c == '\r') {
                if (execute_command(buffer)) break;
                mode = EditorMode::Normal;
                command_buffer.clear();
            } else if (c == 127 || c == 8) {
                if (!command_buffer.empty()) command_buffer.pop_back();
            } else if (c == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, seq, 2) == 2 && seq[0] == '[') {
                    if (seq[1] == 'A' && history_index > 0) {
                        history_index--;
                        command_buffer = command_history[history_index];
                    } else if (seq[1] == 'B' && history_index < (int)command_history.size() - 1) {
                        history_index++;
                        command_buffer = command_history[history_index];
                    }
                }
            } else {
                command_buffer += c;
            }

        } else if (mode == EditorMode::Insert) {
            if (c == 27) {
                mode = EditorMode::Normal;
            } else if (c == 127 || c == 8) {
                buffer.delete_before();
            } else if (c == '\r') {
                buffer.insert('\n');
            } else if (isprint(c)) {
                buffer.insert(c);
            }

        } else {
            // Normal mode
            if (c == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, seq, 2) == 2 && seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': buffer.move_up(); break;
                        case 'B': buffer.move_down(); break;
                        case 'C': buffer.move_right(); break;
                        case 'D': buffer.move_left(); break;
                    }
                }
            } else {
                switch (c) {
                    case ':': command_buffer.clear(); mode = EditorMode::Command; break;
                    case 'i': mode = EditorMode::Insert; break;
                    case 'h': buffer.move_left(); break;
                    case 'l': buffer.move_right(); break;
                    case 'j': buffer.move_down(); break;
                    case 'k': buffer.move_up(); break;
                    case '0': buffer.move_start(); break;
                    case '$': buffer.move_end(); break;
                    case 'x': buffer.delete_char(); break;
                }
            }
        }
    }

    clear_screen();
    return 0;
}