//
// Created by dzuha on 10/09/2024.
//

#ifndef ALPHABET_H
#define ALPHABET_H

#include <string>
#include <vector>

class Alphabet {
private:
    std::vector<std::string> letters;

public:
    explicit Alphabet(std::vector<std::string> letters);
    [[nodiscard]] std::vector<std::string> getLetters() const;
    std::vector<std::string>& _getLetters();
    bool hasLetter(const std::string& letter);
};

#endif //ALPHABET_H
