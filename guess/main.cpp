#include <iostream>
#include <random>

int difficulty()
{
    int d{};
    while (d < 1 || d > 3)
    {
        std::cout << "Choose difficulty (1, 2, 3): ";
        std::cin >> d;
    }
    return d;
}

int maxNumber(int num)
{
    switch(num)
    {
        case 1:
            std::cout << "Guess a number between 0 and 25\n";
            return 25;
        case 2:
            std::cout << "Guess a number between 0 and 50\n";
            return 50;
        default:
            std::cout << "Guess a number between 0 and 100\n";
            return 100;
    }
}

int guess(int max)
{
    int num{};

    while (true)
    {
        std::cout << "Enter a number: ";
        std::cin >> num;

        if (num >= 0 && num <= max)
            return num;

        std::cout << "Please enter a number between 0 and " << max << ".\n";
    }
}

bool isGuessed(int num, int secret)
{
    if (num > secret)
    {
        std::cout << "Secret number is smaller.\n";
        return false;
    } else if (num < secret) {
        std::cout << "Secret number is bigger.\n";
        return false;
    } else {
        std::cout << "You guessed the number!!!\n";
        return true;
    }
}

int main() {
    bool won {false};
    int max = maxNumber(difficulty());

    std::random_device rd;
    std::mt19937 mt{rd()};
    std::uniform_int_distribution<int> number { 0, max };
    int secret = number(mt);

    for (int i{}; i < 5; i++) {
        if (isGuessed(guess(max), secret))
        {
            won = true;
            break;
        }
    }
    if (!won) {
        std::cout << "You failed. The secret number is " << secret << ".\n";
    }

    return 0;
}
