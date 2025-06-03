#include <iostream>
#include <unistd.h>
#include "ui/term.h"
#include "render.h"
#include <termios.h>
#include <sys/ioctl.h>
#include <csignal>
#include <cstdlib>
#include "buffer.h"
#include "command.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

EditorMode mode = EditorMode::NORMAL;
std::string command_buffer;
std::vector<std::string> command_history;
int history_index = 0;


int main(int argc, char** argv) {
    enable_raw_mode();

    std::string initial_contents;
    if (argc >= 2) {
        std::ifstream in(argv[1]);
        if (!in) {
            std::cerr << "Warning: Could not open '" << argv[1] << "'\n";
        } else {
            std::ostringstream ss;
            ss << in.rdbuf();
            initial_contents = ss.str();
        }
    }

    EditorBuffer buffer(initial_contents.c_str());
    int rows, cols;
    get_window_size(rows, cols);

    while (true) {
        render_buffer(buffer, rows, cols);

        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;

        if (mode == EditorMode::COMMAND) {
            if (c == '\r') {
                CommandResult result = run_command(buffer, command_buffer);
                if (result == CommandResult::QUIT) break;
                mode = EditorMode::NORMAL;
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

        } else if (mode == EditorMode::INSERT) {
            if (c == 27) {
                mode = EditorMode::NORMAL;
            } else if (c == 127 || c == 8) {
                buffer.backspace_char();
            } else if (c == '\r') {
                buffer.insert('\n');
            } else if (isprint(c)) {
                buffer.insert(c);
            }

        } else if (mode == EditorMode::NORMAL) {
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
                    case ':': mode = EditorMode::COMMAND; break;
                    case 'i': mode = EditorMode::INSERT; break;
                    case 'h': buffer.move_left(); break;
                    case 'l': buffer.move_right(); break;
                    case 'j': buffer.move_down(); break;
                    case 'k': buffer.move_up(); break;
                    case 's': buffer.move_start(); break;
                    case 'e': buffer.move_end(); break;
                    case 127:
                    case 8: buffer.backspace_char(); break;
                    default: break; // ignore any isprint() here
                }
            }
        }
    }

    term_clear_screen();
    return 0;
}
