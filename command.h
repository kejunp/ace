#pragma once

#include "buffer.h"
#include <string>


enum class CommandResult {
    CONTINUE,
    QUIT,
    ERROR,
};


CommandResult command_q();

CommandResult command_w(EditorBuffer& buf, const std::string& path = "ace.out");

CommandResult command_wq(EditorBuffer& buf, const std::string& path = "ace.out");

CommandResult command_substitute(EditorBuffer& buf, const std::string& pattern, const std::string& replacement);

CommandResult run_command(EditorBuffer& buf, const std::string& input);