#include <fstream>
#include "command.h"

CommandResult command_q(EditorBuffer& buf)
{
    return CommandResult::QUIT;
}

CommandResult command_w(EditorBuffer& buf, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << path << "' for writing.\n";
        return CommandResult::ERROR;
    }

    file << buf.get_string();

    if (!file) {
        std::cerr << "Error: Failed to write to file '" << path << "'.\n";
        return CommandResult::ERROR;
    }

    return CommandResult::CONTINUE;
}