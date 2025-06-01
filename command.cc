#include <fstream>
#include "command.h"

CommandResult command_q(EditorBuffer& buf)
{
    return CommandResult::QUIT;
}

CommandResult command_w(EditorBuffer& buf, const std::string& path) {
    std::ofstream file(path);
    
}
