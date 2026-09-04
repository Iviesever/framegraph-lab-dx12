#include <iostream>
#include <span>
#include <array>

int main() {
    constexpr std::array values{1, 2, 3};
    const std::span view(values);
    if (view.size() != 3 || view.back() != 3) return 1;
    std::cout << "baseline C++23 toolchain smoke passed\n";
}
