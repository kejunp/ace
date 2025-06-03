#include "command.h"
#include "buffer.h"
#include <unordered_map>
#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>

CommandResult command_q() {
    return CommandResult::QUIT;
}

CommandResult command_w(EditorBuffer& buf, const std::string& path) {
    std::string filename = path.empty() ? "ace.out" : path;
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Failed to open file '" << filename << "' for writing.\n";
        return CommandResult::ERROR;
    }
    file << buf.get_string();
    return CommandResult::CONTINUE;
}

CommandResult command_wq(EditorBuffer& buf, const std::string& path) {
    command_w(buf, path);
    return CommandResult::QUIT;
}

CommandResult command_substitute(EditorBuffer& buf, const std::string& pattern, const std::string& replacement) {
    buf.substitute_all(pattern, replacement);
    return CommandResult::CONTINUE;
}

/** TODO: 
 * add more commands
 */

CommandResult run_command(EditorBuffer& buffer, const std::string& input) {
    static const std::unordered_map<std::string, std::function<CommandResult(EditorBuffer&, const std::string&)>> dispatch = {
        {"q", [](EditorBuffer&, const std::string&) { return command_q(); }},
        {"w", command_w},
        {"wq", command_wq},
        {"substitute", [](EditorBuffer& buf, const std::string& input) {
            // Expect format: /pattern/replacement/
            if (input.size() < 3 || input[0] != '/') {
                std::cerr << "Usage: :substitute /pattern/replacement/\n";
                return CommandResult::ERROR;
            }

            size_t second = input.find('/', 1);
            size_t third  = input.find('/', second + 1);

            if (second == std::string::npos) {
                std::cerr << "Usage: :substitute /pattern/replacement/\n";
                return CommandResult::ERROR;
            }

            std::string pattern = input.substr(1, second - 1);
            std::string replacement;

            if (third != std::string::npos) {
                replacement = input.substr(second + 1, third - second - 1);
            } else {
                replacement = input.substr(second + 1);
            }

            return command_substitute(buf, pattern, replacement);
        }}
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
