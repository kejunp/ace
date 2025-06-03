#include <sys/ioctl.h>
#include <string>
#include <vector>
#include <unistd.h>
#include "buffer.h"
#include "term.h"

int get_window_size(int& rows, int& cols);

enum class EditorMode { NORMAL, INSERT, COMMAND };
extern EditorMode mode;
extern std::string command_buffer;
extern std::vector<std::string> command_history;

void render_buffer(EditorBuffer& buf, int term_rows, int term_cols);