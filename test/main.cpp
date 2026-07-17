#include <iostream>

enum Sumer { Ur = 1<<0, Uruk = 1<<1, Kish = 1<<2, Adab = 1<<3, Lagash = 1<<4 };

int main()
{
    // There are different ways to set one bit
    Sumer capital {Sumer(Uruk)}; // Only initialization
    std::cout << "Capital: " << capital << std::endl;
    Sumer capitals = Sumer(Uruk); // Only initialization
    // Initialization and assignment
    capital = Uruk;
    capital = static_cast<Sumer>(2);
    capital = static_cast<Sumer>(1<<2);
    capital = static_cast<Sumer>(0b00010);

    // There are different ways to set several bits
    Sumer cities {Sumer(Ur | Kish)}; // Only initialization
    Sumer ciudades = Sumer(Ur | Kish);
    std::cout << "Ciudades: " << ciudades << std::endl;
    // Initialization and assignment
    cities = static_cast<Sumer>(Ur | Kish);
    cities = static_cast<Sumer>(5);
    cities = static_cast<Sumer>(1<<0 | 1<<2);
    cities = static_cast<Sumer>(0b00101);
    
    // How to query using bitmasks
    Sumer target = static_cast<Sumer>(0b11011);   // Ur, Uruk, Adab, Lagash
    Sumer current = static_cast<Sumer>(0b01110);  // Uruk, Kish, Adab

    // Unscoped enums convert implicitly to int, so casting is optional for output
    std::cout << target << std::endl;                   // 27 (11011) <- target bits to achieve
    std::cout << current << std::endl;                  // 14 (01110) <- current bits
    std::cout << (current & target) << std::endl;       // 10 (01010) <- target bits achieved
    std::cout << (target & ~current) << std::endl;      // 17 (10001) <- remaining target bits to achieve
    std::cout << ((current & target) ^ current) << std::endl; // 4 (00100) - other

    // Comparison logic
    // Unscoped enums allow direct bitwise operations without casting
    std::cout << "Capital: " << capital << ", cities: " << cities << std::endl;
    if ((current & target) == current) {
        std::cout << "Exact match. Target achieved!" << std::endl;
    }
    else if ((current | target) == current) {
        std::cout << "Target achieved!" << std::endl;
    }
    else {
        std::cout << "Target not achieved!" << std::endl;
        std::cout << (target & ~current) << " missing" << std::endl;
    }

    return 0;
}
