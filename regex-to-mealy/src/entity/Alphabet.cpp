
#include <entity/Alphabet.h>
#include <algorithm>  // for std::find

Alphabet::Alphabet(std::vector<std::string> letters)
        : letters(std::move(letters)) {}

[[nodiscard]] std::vector<std::string> Alphabet::getLetters() const {
    return letters;
}
std::vector<std::string>& Alphabet::_getLetters() {
    return letters;
}

bool Alphabet::hasLetter(const std::string& letter)
{
    return std::find(letters.begin(), letters.end(), letter) != letters.end();
}
