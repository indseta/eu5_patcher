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
#include <utility>
#include <vector>

namespace fs = std::filesystem;

// Constants

// Achievement Check
constexpr std::string_view PATTERN =
    "80 ?? ?? ?? ?? ?? 00 75 ?? "
    "80 ?? ?? ?? ?? ?? 00 74 ?? "
    "80 ?? ?? ?? ?? ?? 00 74 ?? "
    "80 ?? ?? ?? ?? ?? 00";
constexpr std::string_view PATTERN_REPLACE = "80 ?? ?? ?? ?? ?? 00 eb";

// Ironman Save and Load
constexpr std::string_view PATTERN2 =
    "74 ?? "
    "?? ?? ?? "
    "ff ?? ?? "
    "e8 ?? ?? ?? ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "74 ?? "
    "32 c0";

constexpr std::string_view PATTERN_REPLACE2 =
    "eb ?? "
    "?? ?? ?? "
    "ff ?? ?? "
    "e8 ?? ?? ?? ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "74 ?? "
    "b0 00";

// Ironman Console
constexpr std::string_view PATTERN3 =
    "00 "
    "75 ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "32 c0 "
    "48 ?? ?? ?? "
    "c3 "
    "b0 01";

constexpr std::string_view PATTERN_REPLACE3 =
    "00 "
    "75 ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "32 c0 "
    "48 ?? ?? ?? "
    "c3 "
    "b0 00";

// Save
constexpr std::string_view PATTERN4 =
    "e8 ?? ?? ?? ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "48 ?? ?? "
    "e8";

constexpr std::string_view PATTERN_REPLACE4 =
    "e8 ?? ?? ?? ?? "
    "80 ?? ?? ?? ?? ?? 09";

// Load
constexpr std::string_view PATTERN5 =
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "83 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "48 ?? ?? "
    "e8";

constexpr std::string_view PATTERN_REPLACE5 =
    "80 ?? ?? ?? ?? ?? 09";

struct PatchDefinition
{
    std::string_view label;
    std::string_view pattern;
    std::string_view replacement;
};

const std::vector<PatchDefinition> PATCH_DEFINITIONS = {
    {"Patch #1", PATTERN, PATTERN_REPLACE},
    {"Patch #2", PATTERN2, PATTERN_REPLACE2},
    {"Patch #3", PATTERN3, PATTERN_REPLACE3},
    {"Patch #4", PATTERN4, PATTERN_REPLACE4},
    {"Patch #5", PATTERN5, PATTERN_REPLACE5},
};

constexpr std::string_view EU5_PATH = "eu5.exe";
constexpr std::string_view EU5_BACKUP_PATH = "eu5.exe.backup";

bool debuge_info = false;

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
 * Apply the replacement bytes at the given offset, printing progress details.
 */
void apply_patch_bytes(std::vector<uint8_t> &data,
                       size_t offset,
                       const std::vector<PatternByte> &replacement,
                       std::string_view label)
{
    std::cout << '\n'
              << label << " found at offset: 0x"
              << std::hex << offset << std::dec << '\n';

    if (debuge_info)
        std::cout << "Applying " << label << "...\n\n";

    for (size_t i = 0; i < replacement.size(); ++i)
    {
        const size_t absolute_pos = offset + i;
        const uint8_t original = data[absolute_pos];
        const auto &pb = replacement[i];

        if (debuge_info)
            std::cout << "0x" << std::hex << std::setfill('0')
                      << std::setw(6) << absolute_pos << ": 0x"
                      << std::setw(2) << static_cast<int>(original) << " -> 0x";

        if (pb.is_wildcard)
        {
            if (debuge_info)
                std::cout << std::setw(2) << static_cast<int>(original)
                          << " (unchanged)";
        }
        else
        {
            if (debuge_info)
                std::cout << std::setw(2) << static_cast<int>(pb.value);

            data[absolute_pos] = pb.value;
        }
        if (debuge_info)
            std::cout << std::dec << '\n';
    }
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
    // Read file
    auto data_opt = read_file(filepath);
    if (!data_opt)
    {
        std::cerr << "Error: Failed to read " << filepath << '\n';
        return 1;
    }
    auto &data = *data_opt;

    std::vector<size_t> offsets;
    offsets.reserve(PATCH_DEFINITIONS.size());
    std::vector<std::vector<PatternByte>> replacements;
    replacements.reserve(PATCH_DEFINITIONS.size());

    // Parse patterns and determine offsets
    for (const auto &patch_def : PATCH_DEFINITIONS)
    {
        const auto pattern = parse_pattern(patch_def.pattern);
        auto offset_opt = find_pattern(data, pattern);
        if (!offset_opt)
        {
            std::cerr << "Error: " << patch_def.label
                      << " not found. Have you patched it before?\n"
                      << "If not, the pattern may need to be updated.\n";
            return 1;
        }

        offsets.push_back(*offset_opt);
        auto replacement = parse_pattern(patch_def.replacement);
        replacements.push_back(std::move(replacement));
    }

    // Create backup only after confirming both patterns exist
    if (!create_backup(filepath, fs::path(EU5_BACKUP_PATH)))
    {
        std::cerr << "Error: Failed to create backup.\n";
        return 1;
    }
    std::cout << "Backup created: " << EU5_BACKUP_PATH << '\n';

    for (size_t i = 0; i < PATCH_DEFINITIONS.size(); ++i)
    {
        apply_patch_bytes(data, offsets[i], replacements[i], PATCH_DEFINITIONS[i].label);
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