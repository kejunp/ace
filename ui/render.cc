#include "render.h"


int get_window_size(int& rows, int& cols) {
    winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) return -1;
    rows = ws.ws_row;
    cols = ws.ws_col;
    return 0;
}

void render_buffer(EditorBuffer& buf, int term_rows, int term_cols) {
    term_clear_screen();

    int row = 1, col = 1;
    int cursor_row = 1, cursor_col = 1;

    EditorBuffer::Node* sentinel = buf.get_head();
    EditorBuffer::Node* cursor = buf.get_cursor();
    EditorBuffer::Node* p = sentinel->next;

    while (p != sentinel && row <= term_rows - 1) {
        // Track position *after* printing this character
        if (p == cursor) {
            if (p->data == '\n') {
                cursor_row = row + 1;
                cursor_col = 1;
            } else if (col == term_cols) {
                cursor_row = row + 1;
                cursor_col = 1;
            } else {
                cursor_row = row;
                cursor_col = col + 1;
            }
        }

        move_cursor(row, col);
        std::cout << (p->data == '\n' ? ' ' : p->data);

        if (p->data == '\n') {
            ++row;
            col = 1;
        } else {
            ++col;
            if (col > term_cols) {
                ++row;
                col = 1;
            }
        }

        p = p->next;
    }

    if (cursor == sentinel) {
        cursor_row = row;
        cursor_col = col;
    }

    // Status bar
    move_cursor(term_rows, 1);
    if (mode == EditorMode::COMMAND) {
        std::cout << ":" << command_buffer << " ";
    } else {
        std::cout << "\x1b[44m-- ACE (" << (mode == EditorMode::INSERT ? "INSERT" : "NORMAL") << ") --\x1b[0m";
    }

    // Move cursor to correct position
    move_cursor(cursor_row, cursor_col);
    std::cout.flush();
}