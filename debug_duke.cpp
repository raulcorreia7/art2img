#include "include/palette.hpp"
#include <iostream>

int main() {
    art2img::Palette palette;
    palette.load_duke3d_default();

    std::cout << "After load_duke3d_default:" << std::endl;
    auto pos_blue = 255 * 3;
    auto pos_green = 255 * 3 + 1;
    auto pos_red = 255 * 3 + 2;

    std::cout << "Data at position 255:" << std::endl;
    std::cout << "  Blue pos " << pos_blue << ": " << (int)palette.data()[pos_blue] << std::endl;
    std::cout << "  Green pos " << pos_green << ": " << (int)palette.data()[pos_green] << std::endl;
    std::cout << "  Red pos " << pos_red << ": " << (int)palette.data()[pos_red] << std::endl;

    std::cout << "Function returns:" << std::endl;
    std::cout << "  get_red(255): " << (int)palette.get_red(255) << std::endl;
    std::cout << "  get_green(255): " << (int)palette.get_green(255) << std::endl;
    std::cout << "  get_blue(255): " << (int)palette.get_blue(255) << std::endl;

    return 0;
}