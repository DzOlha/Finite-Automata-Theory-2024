//
// Created by dzuha on 10/09/2024.
//

#ifndef ALPHABET_H
#define ALPHABET_H

#include <vector>

class Alphabet
{
private:
    std::vector<char> letters;

public:
    explicit Alphabet(std::vector<char> letters);
    [[nodiscard]] std::vector<char> getLetters() const;
    std::vector<char>& _getLetters();
};

#endif //ALPHABET_H
