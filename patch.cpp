/**
 * EU5 Patcher - Enable Achievements Unconditionally
 * Compile: cl /std:c++17 /O2 /EHsc patch.cpp
 */

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

// Constants
constexpr std::string_view PATTERN =
    "80 ?? ?? ?? ?? ?? 00 75 ?? "
    "80 ?? ?? ?? ?? ?? 00 74 ?? "
    "80 ?? ?? ?? ?? ?? 00 74 ?? "
    "80 ?? ?? ?? ?? ?? 00";
constexpr std::string_view PATTERN_REPLACE = "80 ?? ?? ?? ?? ?? 00 eb";
constexpr std::string_view EU5_PATH = "eu5.exe";
constexpr std::string_view EU5_BACKUP_PATH = "eu5.exe.backup";

/**
 * Represents a single byte in a pattern, which can be either
 * a specific value or a wildcard.
 */
struct PatternByte
{
    uint8_t value{0};
    bool is_wildcard{false};

    static PatternByte wildcard() { return {0, true}; }
    static PatternByte exact(uint8_t v) { return {v, false}; }
};

/**
 * Parse a pattern string (e.g., "80 ?? AB") into a vector of PatternBytes.
 */
[[nodiscard]] std::vector<PatternByte> parse_pattern(std::string_view pattern_str)
{
    std::vector<PatternByte> result;
    std::istringstream iss{std::string(pattern_str)};
    std::string token;

    while (iss >> token)
    {
        if (token == "??")
        {
            result.push_back(PatternByte::wildcard());
        }
        else
        {
            result.push_back(PatternByte::exact(
                static_cast<uint8_t>(std::stoi(token, nullptr, 16))));
        }
    }

    return result;
}

/**
 * Find a pattern in data, returning the offset if found.
 * Supports wildcard bytes in the pattern.
 */
[[nodiscard]] std::optional<size_t> find_pattern(
    const std::vector<uint8_t> &data,
    const std::vector<PatternByte> &pattern)
{
    if (pattern.empty() || data.size() < pattern.size())
    {
        return std::nullopt;
    }

    const size_t pattern_len = pattern.size();
    const size_t search_end = data.size() - pattern_len;

    for (size_t i = 0; i <= search_end; ++i)
    {
        bool match = true;

        for (size_t j = 0; j < pattern_len && match; ++j)
        {
            if (!pattern[j].is_wildcard && data[i + j] != pattern[j].value)
            {
                match = false;
            }
        }

        if (match)
        {
            return i;
        }
    }

    return std::nullopt;
}

/**
 * Read entire file into a byte vector.
 */
[[nodiscard]] std::optional<std::vector<uint8_t>> read_file(const fs::path &filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        return std::nullopt;
    }

    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        return std::nullopt;
    }

    return buffer;
}

/**
 * Write byte vector to file.
 */
[[nodiscard]] bool write_file(const fs::path &filepath, const std::vector<uint8_t> &data)
{
    std::ofstream file(filepath, std::ios::binary);
    if (!file)
    {
        return false;
    }

    file.write(reinterpret_cast<const char *>(data.data()),
               static_cast<std::streamsize>(data.size()));
    return file.good();
}

/**
 * Create a backup of the target file.
 */
[[nodiscard]] bool create_backup(const fs::path &source, const fs::path &dest)
{
    std::error_code ec;
    fs::copy_file(source, dest, fs::copy_options::overwrite_existing, ec);
    return !ec;
}

/**
 * Apply the patch to the target file.
 * Returns: 0 = success, 1 = error
 */
[[nodiscard]] int make_patch(const fs::path &filepath)
{
    // Create backup
    if (!create_backup(filepath, fs::path(EU5_BACKUP_PATH)))
    {
        std::cerr << "Error: Failed to create backup.\n";
        return 1;
    }
    std::cout << "Backup created: " << EU5_BACKUP_PATH << '\n';

    // Read file
    auto data_opt = read_file(filepath);
    if (!data_opt)
    {
        std::cerr << "Error: Failed to read " << filepath << '\n';
        return 1;
    }
    auto &data = *data_opt;

    // Parse patterns
    const auto pattern = parse_pattern(PATTERN);
    const auto replace_pattern = parse_pattern(PATTERN_REPLACE);

    // Find pattern
    auto offset_opt = find_pattern(data, pattern);
    if (!offset_opt)
    {
        std::cerr << "Error: Pattern not found. Have you patched it before?\n"
                  << "If not, the pattern may need to be updated.\n";
        return 1;
    }
    const size_t offset = *offset_opt;

    std::cout << "\nPattern found at offset: 0x" << std::hex << offset << std::dec << '\n';
    std::cout << "Applying patch...\n\n";

    // Apply replacement
    for (size_t i = 0; i < replace_pattern.size(); ++i)
    {
        const uint8_t original = data[offset + i];
        const auto &pb = replace_pattern[i];

        std::cout << "0x" << std::hex << std::setfill('0')
                  << std::setw(6) << (offset + i) << ": 0x"
                  << std::setw(2) << static_cast<int>(original) << " -> 0x";

        if (pb.is_wildcard)
        {
            std::cout << std::setw(2) << static_cast<int>(original) << " (unchanged)";
        }
        else
        {
            std::cout << std::setw(2) << static_cast<int>(pb.value);
            data[offset + i] = pb.value;
        }
        std::cout << std::dec << '\n';
    }

    // Write back
    if (!write_file(filepath, data))
    {
        std::cerr << "Error: Failed to write patched file.\n";
        return 1;
    }

    std::cout << "\nEU5 is successfully patched.\n";
    return 0;
}

int main()
{
    const fs::path target_path{EU5_PATH};

    // Check if target exists
    if (!fs::exists(target_path))
    {
        std::cerr << "eu5.exe not found.\n"
                  << "Place this file in .../Europa Universalis V/binaries/\n";
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    // Apply patch
    const int result = make_patch(target_path);

    std::cout << "Press Enter to exit...";
    std::cin.get();

    return result;
}