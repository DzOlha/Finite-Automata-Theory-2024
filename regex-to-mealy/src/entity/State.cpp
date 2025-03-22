#include <entity/State.h>

std::string State::implode(const std::vector<int>& elements, char delimiter)
{
    if (elements.empty())
    {
        return "<empty State>"; // also can add address in memory of State object
    }

    std::string result;
    result += std::to_string(elements[0]);

    for (size_t i = 1; i < elements.size(); ++i)
    {
        result += delimiter;
        result += std::to_string(elements[i]);
    }
    return result;
}

State::State(std::set<int> substates, bool final)
    : substates(std::move(substates)), final(final)
{
}

bool State::isFinal() const
{
    return final;
}

void State::setFinal(bool final)
{
    this->final = final;
}

std::string State::getName() const
{
    if (!substates.empty())
    {
        return implode(std::vector<int>(this->substates.begin(), this->substates.end()), ',');
    }
    return secondaryName;
}

/**
 * Checks if state is atomic (has only one substate)
 */
bool State::isAtomic() const
{
    return this->substates.size() == 1;
}

void State::setSubstates(const std::set<int>& newSubstates)
{
    this->substates = newSubstates;
}

const std::set<int> State::getSubstates() const
{
    return this->substates;
}

bool State::operator==(const State& other) const
{
    return substates == other.substates && final == other.final;
}

namespace std
{
    size_t hash<State>::operator()(const State& state) const
    {
        size_t hashValue = 0;
        for (char c : state.getName())
        {
            hashValue ^= std::hash<char>{}(c) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
        }
        hashValue ^= std::hash<bool>{}(state.isFinal());
        return hashValue;
    }
}
