#pragma once

#include "buffer.h"
#include <string>


enum class CommandResult {
    CONTINUE,
    QUIT,
    ERROR,
};


CommandResult command_q(EditorBuffer& buf);

CommandResult command_w(EditorBuffer& buf, const std::string& path = "ace.out");