#ifndef TOOLS
#define TOOLS

#include <sys/wait.h>
#include <memory>
#include <mutex>
#include <string>
#include <SDL3/SDL.h>
#include <cctype>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "../json/json.hpp"

using json = nlohmann::json;

// Branchless hex char to int
inline int hex_char_2_int(char c) {
    c = std::toupper(static_cast<unsigned char>(c)); // Still uses a function but avoids branching
    int is_digit = (c >= '0') & (c <= '9');
    int is_alpha = (c >= 'A') & (c <= 'F');
    return is_digit * (c - '0') + is_alpha * (10 + (c - 'A'));
}

// Branchless RGB conversion from hex string
inline SDL_Color hex_2_rgb(const std::string& hex_in) {
    SDL_Color color = {0, 0, 0, 255}; // Default alpha = 255

    std::string hex = hex_in;
    int start = (hex[0] == '#'); // Skip '#' if present
    hex = hex.substr(start);

    // Expand 3-digit hex code to 6-digit format without branching
    std::string expanded_hex = hex;
    if (hex.length() == 3) {
        expanded_hex = std::string() +
                       hex[0] + hex[0] +
                       hex[1] + hex[1] +
                       hex[2] + hex[2];
    }

    // Only process if length is exactly 6
    // Use a ternary to assign or not based on size
    int valid = (expanded_hex.length() == 6);

    color.r = valid * (hex_char_2_int(expanded_hex[0]) * 16 + hex_char_2_int(expanded_hex[1]));
    color.g = valid * (hex_char_2_int(expanded_hex[2]) * 16 + hex_char_2_int(expanded_hex[3]));
    color.b = valid * (hex_char_2_int(expanded_hex[4]) * 16 + hex_char_2_int(expanded_hex[5]));

    return color;
}

inline std::string run_command(const char* cmd) {
    int buff_size = 256;
    char buff[buff_size + 1];
    std::string str;
    
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return "";
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return "";
    }

    if (pid == 0) {
        // Child process
        close(fd[0]); // Close read end
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        close(fd[1]);

        execl("/bin/sh", "sh", "-c", cmd, (char*)nullptr);
        perror("execl"); // Only happens on error
        exit(1);
    }

    // Parent process
    close(fd[1]); // Close write end
    ssize_t count;
    while ((count = read(fd[0], buff, buff_size)) > 0) {
        str.append(buff, count);
    }
    close(fd[0]);

    waitpid(pid, nullptr, 0);
    return str;
}

inline std::string query_gemini(const std::string& prompt) {
    const std::string api_key = "ADD-YOUR-OWN";

    // Properly escape JSON payload for shell
    std::string json_data =
        "{\"contents\":[{\"parts\":[{\"text\":\"" + prompt + "\"}]}]}";

    std::string cmd =
        "curl -s \"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent\" "
        "-H \"Content-Type: application/json\" "
        "-H \"X-goog-api-key: " + api_key + "\" "
        "-X POST "
        "-d '" + json_data + "'";

    std::string result;
    char buffer[256];

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "ERROR: Failed to run curl command.\n";
        return "";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    int exit_code = pclose(pipe);
    if (exit_code != 0) {
        std::cerr << "ERROR: curl exited with code " << exit_code << "\n";
    }

    if (result.empty()) {
        std::cerr << "ERROR: Gemini API returned an empty response.\n";
    }

    return result;
}

inline std::string extract_text(const std::string& json_str) {
    if (json_str.empty()) {
        std::cerr << "ERROR: extract_text received empty string.\n";
        return "";
    }

    try {
        auto j = json::parse(json_str);

        // Optional: sanity check
        if (!j.contains("candidates") || j["candidates"].empty()) {
            std::cerr << "ERROR: JSON does not contain valid 'candidates'.\n";
            return "";
        }

        return j["candidates"][0]["content"]["parts"][0]["text"];
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        std::cerr << "Raw input: " << json_str << "\n";
        return "";
    }
}

#endif // TOOLS
