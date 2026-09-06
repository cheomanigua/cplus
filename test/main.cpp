#include <array>
#include <iostream>
#include <unordered_map>
#include <vector>

enum class Day
{
    Monday = 1,
    Tuesday,
    Wednesday
};

int main()
{
    Day day {Day::Wednesday};

    if (day == Day::Wednesday)
    {
        std::cout << "It's Wednesday\n";
    } else {
        std::cout << "It's Tuesday\n";
    }

    std::vector<int> numbers = {10, 20, 30, 20, 15, 26, 20};
    for (const auto& number : numbers) {
        std::cout << number << "\n";
    }
    numbers.erase(std::remove(numbers.begin(), numbers.end(), 20), numbers.end());
    std::cout << "------\n";
    for (const auto& number : numbers) {
        std::cout << number << "\n";
    }
    constexpr std::size_t vsize {50'000};
    std::vector<int> vvalues {};
    std::array<int, vsize> avalues {};
    std::unordered_map<int, int> mvalues {};
    int sum1 {};
    int sum2 {};
    int sum3 {};

    for (std::size_t i = 1; i < vsize; ++i)
    {
        vvalues.push_back(static_cast<int>(i));
        avalues[i] = static_cast<int>(i);
        mvalues.emplace(vvalues.back(), avalues[i]);
    }

    //for (std::size_t i = 0; i < vvalues.size(); ++i)
    for (const auto& value : vvalues)
    {
        //sum1 += vvalues[i];
        sum1 += value;
    }

    //for (std::size_t i = 0; i < avalues.size(); ++i)
    for (const auto& value : avalues)
    {
        //sum2 += avalues[i];
        sum2 += value;
    }

    for (const auto& [key, value] : mvalues)
    {
        sum3 += value;
    }

    std::cout << sum1 << ' ' << sum2 << ' ' << sum3 << '\n';
}

