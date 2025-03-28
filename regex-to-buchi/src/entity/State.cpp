#include <stdexcept>
#include <entity/State.h>

#include <utility>

// returns string with elements of given vector, delimited by given delimiter
std::string State::vectorToString(const std::vector<int>& elements, char delimiter) {
    if (elements.empty()) {
        return "<empty>"; // also can add address in memory of State object
    }

    std::string result;
    result += std::to_string(elements[0]);

    for (size_t i = 1; i < elements.size(); ++i) {
        result += delimiter;
        result += std::to_string(elements[i]);
    }
    return result;
}

// returns string with elements of given set, delimited by given delimiter
std::string State::setToString(const std::set<int>& elements, char delimiter) {
    return vectorToString(std::vector<int>{elements.begin(), elements.end()}, delimiter);
}

State::State(std::vector<std::set<int>> elementaryStates, bool final) :
    elementaryStates{std::move(elementaryStates)},
    final {final}
{}

// generate from elementary State objects
State::State(const std::vector<std::shared_ptr<State>>& elementaryStates, bool final) :
    final{final}
{
    for(const auto& state : elementaryStates) { // check if input state are elementary
        if(!state->isElementary()) {
            throw std::invalid_argument("State is not an elementary state");
        }
    }

    for(const auto& state : elementaryStates) { // check if input state are elementary
        this->elementaryStates.push_back(state->getElementaryStates()[0]);
    }
}


void State::setSecondaryName(const std::string &secondary_name) {
    secondaryName = secondary_name;
}

std::string State::getSecondaryName() const {
    return this->secondaryName;
}

bool State::isFinal() const {
    return this->final;
}

void State::setFinal(bool final) {
    this->final = final;
}


std::string State::getName() const {
    if(!elementaryStates.empty()) {
        std::string result;

        result += "{" + setToString(this->elementaryStates[0], ',') + "}";
        for (std::size_t i = 1; i < this->elementaryStates.size(); ++i) {
            result += ',';
            result += "{" + setToString(this->elementaryStates[i], ',') + "}";
        }

        if(this->elementaryStates.size() == 1) { // avoid printing extra brackets []
//            return result;
        }
        return "[" + result + "]";
    }
    if(!secondaryName.empty()) {
        return secondaryName;
    }
    return "[]";
}

/**
 * Checks if state is elementary (has only one Elementary State)
 */
bool State::isElementary() const {
    return this->elementaryStates.size() == 1;
}


void State::setElementaryStates(const std::vector<std::set<int>>& newElementaryStates) {
    this->elementaryStates = newElementaryStates;
}

std::vector<std::set<int>> State::getElementaryStates() const {
    return this->elementaryStates;
}

bool State::operator==(const State& other) const {
    return this->elementaryStates == other.elementaryStates &&
        this->final == other.final;
}

namespace std {
    size_t hash<State>::operator()(const State& state) const {
        size_t hashValue = 0;
        for (char c : state.getName()) {
            hashValue ^= std::hash<char>{}(c) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
        }
        hashValue ^= std::hash<bool>{}(state.isFinal());
        return hashValue;
    }
}
