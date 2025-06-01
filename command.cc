#include "command.h"
#include "buffer.h"
#include <unordered_map>
#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>

CommandResult command_q(EditorBuffer&) {
    return CommandResult::QUIT;
}

CommandResult command_w(EditorBuffer& buf, const std::string& arg) {
    std::string filename = arg.empty() ? "ace.out" : arg;
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Failed to open file '" << filename << "' for writing.\n";
        return CommandResult::ERROR;
    }
    file << buf.get_string();
    return CommandResult::CONTINUE;
}

/** TODO: 
 * add more commands
 * e.g. :wq, :substitute, etc.
 */

CommandResult run_command(EditorBuffer& buffer, const std::string& input) {
    static const std::unordered_map<std::string, std::function<CommandResult(EditorBuffer&, const std::string&)>> dispatch = {
        {"q", [](EditorBuffer& buf, const std::string&) { return command_q(buf); }},
        {"w", command_w}
    };

    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    std::string arg;
    std::getline(iss, arg);
    if (!arg.empty() && arg[0] == ' ') arg.erase(0, 1);  // trim leading space

    auto it = dispatch.find(cmd);
    if (it != dispatch.end()) {
        return it->second(buffer, arg);
    }

    std::cerr << "Unknown command: " << cmd << "\n";
    return CommandResult::ERROR;
}
