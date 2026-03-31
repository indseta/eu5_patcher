#include <iostream>
#include <cstdint>

#include <patch.hpp>

int32_t main() {
    const auto target_path_opt = locate_eu5();
    if (!target_path_opt)
    {
        std::cerr << "eu5.exe not found.\n"
                  << "Place this file in .../Europa Universalis V/binaries/\n";
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    const auto &target_path = *target_path_opt;
    std::cout << "Path: " << target_path << '\n';

    // Apply patch
    const int result = make_patch(target_path);

    std::cout << "Press Enter to exit...";
    std::cin.get();

    return result;
}