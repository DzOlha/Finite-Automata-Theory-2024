
#include <entity/Alphabet.h>

Alphabet::Alphabet(std::vector<char> letters)
        : letters(std::move(letters)) {}

[[nodiscard]] std::vector<char> Alphabet::getLetters() const {
    return letters;
}
std::vector<char>& Alphabet::_getLetters() {
    return letters;
}
