#include <entity/Automaton.h>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <utility>

// Default constructor
Automaton::Automaton()
    : alphabet(std::make_unique<Alphabet>(std::vector<char>{})),
      initialState(nullptr),
      transitionsRelation(std::make_unique<TransitionsRelation>(TransitionsRelation::TransitionMap()))
{
}

// Parameterized constructor
Automaton::Automaton(std::unique_ptr<Alphabet> alphabet,
                     std::shared_ptr<State> initialState,
                     std::vector<std::shared_ptr<State>> states,
                     std::vector<std::shared_ptr<State>> finalStates,
                     TransitionsRelation::TransitionMap transitionMap)
    : alphabet(std::move(alphabet)),
      initialState(std::move(initialState)),
      states(std::move(states)),
      finalStates(std::move(finalStates)),
      transitionsRelation(std::make_unique<TransitionsRelation>(std::move(transitionMap)))
{
}

#include <entity/Automaton.h>

std::unique_ptr<Automaton> Automaton::clone() const
{
    // Clone the alphabet using the copy constructor or clone method
    std::unique_ptr<Alphabet> newAlphabet = std::make_unique<Alphabet>(*alphabet);

    // Shared pointers can be copied directly for states
    std::shared_ptr<State> newInitialState = initialState;

    std::vector<std::shared_ptr<State>> newStates = states; // shallow copy
    std::vector<std::shared_ptr<State>> newFinalStates = finalStates; // shallow copy

    // Clone the transitions relation
    TransitionsRelation::TransitionMap oldTransitions = transitionsRelation->getTransitions();
    std::unique_ptr<TransitionsRelation> newTransitionsRelation = std::make_unique<TransitionsRelation>(oldTransitions);

    // Create a new automaton with the cloned data
    return std::make_unique<Automaton>(
        std::move(newAlphabet),
        newInitialState,
        newStates,
        newFinalStates,
        newTransitionsRelation->getTransitions()
    );
}


[[nodiscard]] std::shared_ptr<State> Automaton::getInitialStateShared() const
{
    return this->initialState;
}

void Automaton::print() const
{
    // Print alphabet
    std::cout << "Alphabet: ";
    for (char letter : alphabet->getLetters())
    {
        std::cout << letter << ' ';
    }
    std::cout << '\n';

    // Print initial state
    if (initialState)
    {
        std::cout << "Initial State: " << initialState->getName() << '\n';
    }

    // Print final states
    std::cout << "Final States: ";
    for (const auto& state : finalStates)
    {
        std::cout << state->getName() << ' ';
    }
    std::cout << '\n';

    // Print all states
    std::cout << "States: ";
    for (const auto& state : states)
    {
        std::cout << state->getName() << ' ';
    }
    std::cout << '\n';

    // Print transitions
    std::cout << "Transitions:\n";
    for (const auto& [key, nextStates] : transitionsRelation->getTransitions())
    {
        const auto& fromState = key.first;
        char input = key.second;
        std::cout << fromState->getName() << " --" << input << "--> ";
        for (const auto& toState : nextStates)
        {
            std::cout << toState->getName() << ' '; // Use '->' if toState is shared_ptr
        }
        std::cout << '\n';
    }
}


// Getters
const Alphabet* Automaton::getAlphabet() const
{
    return alphabet.get();
}

Alphabet* Automaton::_getAlphabet() const
{
    return alphabet.get();
}

const State* Automaton::getInitialState() const
{
    return initialState.get();
}

const std::vector<std::shared_ptr<State>>& Automaton::getStates() const
{
    return states;
}

const std::vector<std::shared_ptr<State>>& Automaton::getFinalStates() const
{
    return finalStates;
}

const TransitionsRelation* Automaton::getTransitionsRelation() const
{
    return transitionsRelation.get();
}

TransitionsRelation* Automaton::_getTransitionsRelation() const
{
    return transitionsRelation.get();
}

// Setters
void Automaton::setAlphabet(std::unique_ptr<Alphabet> newAlphabet)
{
    alphabet = std::move(newAlphabet);
}

void Automaton::setInitialState(std::shared_ptr<State> newInitialState)
{
    initialState = std::move(newInitialState);
}

void Automaton::setStates(const std::vector<std::shared_ptr<State>>& newStates)
{
    states = newStates;
}

void Automaton::setFinalStates(const std::vector<std::shared_ptr<State>>& newFinalStates)
{
    finalStates = newFinalStates;
}

void Automaton::setTransitionsRelation(std::unique_ptr<TransitionsRelation> newTransitionsRelation)
{
    transitionsRelation = std::move(newTransitionsRelation);
}


/**
 * Returns state that comprises given substates. If automata does not contain such state, empty pointer is returned
 */
std::shared_ptr<State> Automaton::findStateBySubstates(const std::set<int>& substates) const
{
    for (const std::shared_ptr<State>& s : states)
    {
        if (substates == s->getSubstates())
        {
            return s;
        }
    }
    return std::shared_ptr<State>{};
}

/**
 * Check if each transition has maximum 1 end state
 */
bool Automaton::isDeterministic() const
{
    for (const auto& [key, nextStates] : this->transitionsRelation->getTransitions())
    {
        if (nextStates.size() > 1)
        {
            return false;
        }
    }
    return true;
}
