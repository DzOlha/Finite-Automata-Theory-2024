#include <algorithm>
#include <entity/Automaton.h>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <utility>

// Default constructor
Automaton::Automaton()
    : alphabet(std::make_unique<Alphabet>(std::vector<std::string>{})),
      initialState(nullptr),
      transitionsRelation(std::make_unique<TransitionsRelation>(TransitionsRelation::TransitionMap()))
{
}

// Parameterized constructor
Automaton::Automaton(std::unique_ptr<Alphabet> alphabet,
                     std::shared_ptr<State> initialState,
                     std::vector<std::shared_ptr<State>> states,
                     std::vector<std::shared_ptr<State>> finalStates,
                     TransitionsRelation::TransitionMap transitionMap,
                     const std::unordered_map<
                         std::pair<std::shared_ptr<State>, std::string>, std::vector<std::string>, PairHash
                     >& outSymbolsMap,
                     std::unique_ptr<Alphabet> outAlphabet
)
    : alphabet(std::move(alphabet)),
      initialState(std::move(initialState)),
      states(std::move(states)),
      finalStates(std::move(finalStates)),
      transitionsRelation(std::make_unique<TransitionsRelation>(std::move(transitionMap))),
      outSymbolsRelation(outSymbolsMap),
      outAlphabet(std::move(outAlphabet))
{
}

/**
 * Function that returns all possible outputs based on the given input string
 * @param input
 * @param reverse
 * @return
 */
std::vector<std::string> Automaton::getAllOutputsByInput(const std::string& input, bool reverse)
{
    // Store multiple current states as we can be in more than one state in a non-deterministic automaton
    std::vector<std::shared_ptr<State>> currentStates = {initialState};
    size_t size = input.length();
    int i = 0;

    std::vector<std::string> possibleOutputs; // This will store all possible outputs

    while (i < size)
    {
        // Extract a symbol from the input
        std::string symbol;
        int symbolLength = 1;

        // Check for multi-character symbols (assuming max length is 2)
        for (int len = 1; len <= 2 && i + len <= size; ++len)
        {
            std::string potentialSymbol = input.substr(i, len);
            if (alphabet->hasLetter(potentialSymbol))
            {
                symbol = potentialSymbol;
                symbolLength = len;
            }
        }

        if (symbol.empty())
        {
            throw std::runtime_error("Current substring is not a part of the input alphabet: " + input.substr(i, 1));
        }

        // Track new states and possible outputs for the next iteration
        std::vector<std::shared_ptr<State>> newStates;
        std::vector<std::string> newOutputs;

        // Iterate over all current states (because it's non-deterministic, there may be multiple)
        for (auto& currentState : currentStates)
        {
            // Get all possible next states and corresponding outputs
            auto states = transitionsRelation->move(currentState, symbol);
            auto outs = outSymbolsRelation[{currentState, symbol}];

            // For each state and output pair
            for (size_t j = 0; j < states.size(); ++j)
            {
                newStates.push_back(states[j]);

                // Add this output to all previous possible outputs (non-deterministic, so multiple outputs)
                for (const auto& previousOutput : possibleOutputs)
                {
                    for (const auto& out : outs)
                    {
                        newOutputs.push_back(previousOutput + out); // Append to all previous outputs
                    }
                }

                // If we are at the beginning or if this is the first output, initialize the outputs
                if (possibleOutputs.empty())
                {
                    for (const auto& out : outs)
                    {
                        newOutputs.push_back(out); // Append to all previous outputs
                    }
                }
            }
        }

        // Update current states and possible outputs
        currentStates = newStates;
        possibleOutputs = newOutputs;

        // Move to the next symbol
        i += symbolLength;
    }

    // Reverse the outputs if the flag is set
    if (reverse)
    {
        for (auto& out : possibleOutputs)
        {
            std::reverse(out.begin(), out.end());
        }
    }

    return possibleOutputs;
}

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
        newTransitionsRelation->getTransitions(),
        outSymbolsRelation
    );
}


[[nodiscard]] std::shared_ptr<State> Automaton::getInitialStateShared() const
{
    return this->initialState;
}

void Automaton::print() const
{
    // Print alphabet
    std::cout << "Input Alphabet: ";
    for (const std::string& letter : alphabet->getLetters())
    {
        std::cout << letter << ' ';
    }
    std::cout << '\n';

    // Print out alphabet
    std::cout << "Output Alphabet: ";
    if (outAlphabet)
    {
        for (const std::string& letter : outAlphabet->getLetters())
        {
            std::cout << letter << ' ';
        }
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
        const std::string& input = key.second;

        // Retrieve the corresponding output symbols for the current transition
        auto outSymbolIt = outSymbolsRelation.find(key);

        // Print the transition in the desired format
        // std::cout << fromState->getName() << " -- " << input << " --> ";
        std::cout << fromState->getName() << " -- " << input << " / ";

        // Print the output symbols
        std::cout << "[";
        if (outSymbolIt != outSymbolsRelation.end())
        {
            const auto& outSymbols = outSymbolIt->second;
            for (size_t j = 0; j < outSymbols.size(); ++j)
            {
                std::cout << outSymbols[j];
                if (j < outSymbols.size() - 1)
                {
                    std::cout << ", "; // Comma between output symbols
                }
            }
        }
        std::cout << "] --> ";

        // Print destination states
        for (size_t i = 0; i < nextStates.size(); ++i)
        {
            std::cout << nextStates[i]->getName();
            if (i < nextStates.size() - 1)
            {
                std::cout << ", "; // Comma between states
            }
        }
        std::cout << '\n'; // New line after each transition
    }

    //print outputs
}


void Automaton::printMoore() const
{
    // Print alphabet
    std::cout << "Input Alphabet: ";
    for (const std::string& letter : alphabet->getLetters())
    {
        std::cout << letter << ' ';
    }
    std::cout << '\n';

    std::cout << "Output Alphabet: ";
    for (const std::string& letter : outAlphabet->getLetters())
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
        std::cout << state->getName() << "  ;  ";
    }
    std::cout << '\n';

    // Print transitions
    std::cout << "Transitions:\n";
    for (const auto& [key, nextStates] : transitionsRelation->getTransitions())
    {
        const auto& fromState = key.first;
        const std::string& input = key.second;

        // Retrieve the corresponding output symbols for the current transition
        auto outSymbolIt = outSymbolsRelation.find(key);

        // Print the transition in the desired format
        std::cout << fromState->getName() << " -- " << input << " --> ";


        // Collect all toState names to print in a single output
        std::set<std::string> toStateNames; // Use a set to avoid duplicates
        for (const auto& toState : nextStates)
        {
            toStateNames.insert(toState->getName());
        }

        // Print all the state names together
        for (auto it = toStateNames.begin(); it != toStateNames.end();)
        {
            std::cout << *it;
            ++it;
            if (it != toStateNames.end())
            {
                std::cout << ", "; // Comma between states
            }
        }
        std::cout << '\n'; // New line after each transition
    }


    std::unordered_map<std::pair<std::shared_ptr<State>, std::string>, std::vector<std::string>, PairHash>
        outTransitions;
    outTransitions = getOutSymbolsRelation();
    std::cout << "Output: \n";

    for (const auto& [key, values] : outTransitions)
    {
        //std::string out = outTransitions[std::make_pair(state, std::string())].back();
        std::cout << key.first->getSecondaryName() << "  -->  " << values.front() << std::endl;
    }
}

// folowing function identical to function print which excelent for representation automaton in text
const std::unordered_map<std::string, std::vector<std::string>> Automaton::getAutomatonInText() const
{
    std::vector<std::string> inputAlphabet_text;
    std::vector<std::string> outputAlphabet_text;
    std::vector<std::string> initialState_text;
    std::vector<std::string> finalStates_text;
    std::vector<std::string> states_text;
    std::vector<std::string> transitions_text;
    std::unordered_map<std::string, std::vector<std::string>> out;

    for (const std::string& letter : alphabet->getLetters())
        inputAlphabet_text.push_back(letter);

    for (const std::string& letter : outAlphabet->getLetters())
        outputAlphabet_text.push_back(letter);

    if (initialState)
    {
        initialState_text.push_back(initialState->getName());
        out["initialState"] = initialState_text;
    }

    for (const auto& state : finalStates)
        finalStates_text.push_back(state->getName());

    for (const auto& state : states)
        states_text.push_back(state->getName());

    for (const auto& [key, nextStates] : transitionsRelation->getTransitions())
    {
        const auto& fromState = key.first;
        const std::string& input = key.second;

        std::string new_transition{""};

        auto outSymbolIt = outSymbolsRelation.find(key);

        new_transition += fromState->getName() + "/" + input + "/";

        std::vector<std::string> outSymbolsForState;
        if (outSymbolIt != outSymbolsRelation.end())
        {
            outSymbolsForState = outSymbolIt->second;
        }

        for (size_t j = 0; j < outSymbolsForState.size(); ++j)
        {
            new_transition += outSymbolsForState[j];
        }
        new_transition += "/";
        std::set<std::string> toStateNames;
        for (const auto& toState : nextStates)
        {
            toStateNames.insert(toState->getName());
        }

        for (auto it = toStateNames.begin(); it != toStateNames.end();)
        {
            new_transition += *it;
            ++it;
        }
        transitions_text.push_back(new_transition);
    }

    out["inputAlphabet"] = inputAlphabet_text;
    out["outputAlphabet"] = outputAlphabet_text;
    out["finalStates"] = finalStates_text;
    out["states"] = states_text;
    out["transitions"] = transitions_text;

    return out;
}

// Getters
const Alphabet* Automaton::getAlphabet() const
{
    return alphabet.get();
}

const Alphabet* Automaton::getOutAlphabet() const
{
    return outAlphabet.get();
}


Alphabet* Automaton::getOutputAlphabet() const
{
    return outAlphabet.get();
}

Alphabet* Automaton::_getAlphabet() const
{
    return alphabet.get();
}

Automaton::OutputSymbolMap Automaton::getOutputFunction() const
{
    return outSymbolsRelation;
}

void Automaton::setOutputFunction(const Automaton::OutputSymbolMap& out)
{
    this->outSymbolsRelation = out;
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

const std::unordered_map<std::pair<std::shared_ptr<State>, std::string>, std::vector<std::string>, PairHash
>& Automaton::getOutSymbolsRelation() const
{
    return outSymbolsRelation;
}

std::vector<std::string> Automaton::getOutputOfTransition(const std::shared_ptr<State>& current,
                                                          const std::string& input) const
{
    const std::pair<std::shared_ptr<State>, std::string> key = std::make_pair(current, input);
    auto it = outSymbolsRelation.find(key);
    if (it != outSymbolsRelation.end())
    {
        return it->second;
    }
    return {}; // Return an empty vector if no such transition exists
}

// checks if given word moves automaton from initial state to one of final states
// word is vector of elements of input alphabet
// assumes automaton is deterministic (takes the first possible state), and has all necessary transitions
bool Automaton::doesAcceptWord(const std::vector<std::string>& word) const
{
    if (word.empty())
    {
        throw std::invalid_argument("Input word must not be empty");
    }

    std::shared_ptr<State> currentState = this->initialState;

    for (const std::string& symbol : word)
    {
        std::vector<std::shared_ptr<State>> nextStates = transitionsRelation->move(currentState, symbol);
        if (nextStates.empty())
        {
            //            throw std::invalid_argument("Automaton does not have all transitions");
            return false; // if no transition, automaton can't accept word
        }
        currentState = nextStates[0];
    }

    // check if currentState is final
    for (const std::shared_ptr<State> fs : finalStates)
    {
        if (fs == currentState)
        {
            return true;
        }
    }
    return false;
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
