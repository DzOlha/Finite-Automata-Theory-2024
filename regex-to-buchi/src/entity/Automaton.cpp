#include <algorithm>
#include <entity/Automaton.h>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <utility>

// Default constructor
Automaton::Automaton()
    : alphabet(std::make_unique<Alphabet>(std::vector<std::string>{})),
      initialStates{},
      transitionsRelation(std::make_unique<TransitionsRelation>(TransitionsRelation::TransitionMap()))
    {}

// Parameterized constructor
Automaton::Automaton(std::unique_ptr<Alphabet> alphabet,
                   std::vector<std::shared_ptr<State>> initialStates,
                   std::vector<std::shared_ptr<State>> states,
                   std::vector<std::shared_ptr<State>> finalStates,
                   TransitionsRelation::TransitionMap transitionMap,
                   const std::unordered_map<
                        std::pair<std::shared_ptr<State>, std::string>, std::vector<std::string>, PairHash
                    >& outSymbolsMap,
                    std::unique_ptr<Alphabet> outAlphabet
                    )
    : alphabet(std::move(alphabet)),
      initialStates(std::move(initialStates)),
      states(std::move(states)),
      finalStates(std::move(finalStates)),
      transitionsRelation(std::make_unique<TransitionsRelation>(std::move(transitionMap))),
      outSymbolsRelation(outSymbolsMap),
      outAlphabet(std::move(outAlphabet))
{}

Automaton::Automaton(const Automaton &other)
    : alphabet(std::make_unique<Alphabet>(*other.alphabet))
    , outAlphabet(std::make_unique<Alphabet>(*other.outAlphabet))
    , initialStates(other.initialStates)
    , states(other.states)
    , finalStates(other.finalStates)
    , transitionsRelation(std::make_unique<TransitionsRelation>(*other.transitionsRelation))
    , outSymbolsRelation(other.outSymbolsRelation) {};


std::unique_ptr<Automaton> Automaton::clone() const {
    // Clone the alphabet using the copy constructor or clone method
    std::unique_ptr<Alphabet> newAlphabet = std::make_unique<Alphabet>(*alphabet);

    // Shared pointers can be copied directly for states
    std::vector<std::shared_ptr<State>> newInitialStates = this->initialStates;

    std::vector<std::shared_ptr<State>> newStates = states; // shallow copy
    std::vector<std::shared_ptr<State>> newFinalStates = finalStates; // shallow copy

    // Clone the transitions relation
    TransitionsRelation::TransitionMap oldTransitions = transitionsRelation->getTransitions();
    std::unique_ptr<TransitionsRelation> newTransitionsRelation = std::make_unique<TransitionsRelation>(oldTransitions);

    // Create a new automaton with the cloned data
    return std::make_unique<Automaton>(
        std::move(newAlphabet),
        newInitialStates,
        newStates,
        newFinalStates,
        newTransitionsRelation->getTransitions(),
        outSymbolsRelation
    );
}


void Automaton::print() const {
    // Print alphabet
    std::cout << "Input Alphabet: ";
    for (const std::string& letter : alphabet->getLetters()) {
        std::cout << letter << ' ';
    }
    std::cout << '\n';

    // Print out alphabet
    std::cout << "Output Alphabet: ";
    if(outAlphabet) {
        for (const std::string& letter : outAlphabet->getLetters()) {
            std::cout << letter << ' ';
        }
    }
    std::cout << '\n';


    // Print initial states
    std::cout << "Initial states: ";
    for (const auto& state : this->initialStates) {
        std::cout << state->getName() << ' ';
    }
    std::cout << '\n';


    // Print final states
    std::cout << "Final States: ";
    for (const auto& state : finalStates) {
        std::cout << state->getName() << ' ';
    }
    std::cout << '\n';

    // Print all states
    std::cout << "States: ";
    for (const auto& state : states) {
        std::cout << state->getName() << ' ';
    }
    std::cout << '\n';

    // Print transitions
    std::cout << "Transitions:\n";
    for (const auto& [key, nextStates] : transitionsRelation->getTransitions()) {
        const auto& fromState = key.first;
        const std::string& input = key.second;

        // Retrieve the corresponding output symbols for the current transition
        auto outSymbolIt = outSymbolsRelation.find(key);

        // Print the transition in the desired format
        std::cout << fromState->getName() << " -- " << input << " --> ";
        //std::cout << fromState->getName() << " -- " << input << " / ";

        // Print the output symbols
        // std::cout << "[";
        // if (outSymbolIt != outSymbolsRelation.end()) {
        //     const auto& outSymbols = outSymbolIt->second;
        //     for (size_t j = 0; j < outSymbols.size(); ++j) {
        //         std::cout << outSymbols[j];
        //         if (j < outSymbols.size() - 1) {
        //             std::cout << ", ";  // Comma between output symbols
        //         }
        //     }
        // }
        // std::cout << "] --> ";

        // Print destination states
        for (size_t i = 0; i < nextStates.size(); ++i) {
            std::cout << nextStates[i]->getName();
            if (i < nextStates.size() - 1) {
                std::cout << ", ";  // Comma between states
            }
        }
        std::cout << '\n';  // New line after each transition
    }

    //print outputs

}



// folowing function identical to function print which excellent for representation automaton in text
const std::unordered_map<std::string, std::vector<std::string>> Automaton::getAutomatonInText() const{
    std::vector<std::string> inputAlphabet_text;
    std::vector<std::string> outputAlphabet_text;
    std::vector<std::string> initialStates_text;
    std::vector<std::string> finalStates_text;
    std::vector<std::string> states_text;
    std::vector<std::string> transitions_text;
    std::unordered_map<std::string, std::vector<std::string>> out;
    
    for (const std::string& letter : alphabet->getLetters()) 
        inputAlphabet_text.push_back(letter);

    for (const std::string& letter : outAlphabet->getLetters())
        outputAlphabet_text.push_back(letter);
    

    for (const auto& state : initialStates)
        initialStates_text.push_back(state->getName());

    for (const auto& state : finalStates) 
        finalStates_text.push_back(state->getName());

    for (const auto& state : states) 
        states_text.push_back(state->getName());

    for (const auto& [key, nextStates] : transitionsRelation->getTransitions()) {
        const auto& fromState = key.first;
        const std::string& input = key.second;

        std::string new_transition {""};

        auto outSymbolIt = outSymbolsRelation.find(key);

        new_transition += fromState->getName() + "/" + input +"/";

        std::vector<std::string> outSymbolsForState;
        if (outSymbolIt != outSymbolsRelation.end()){
            outSymbolsForState = outSymbolIt->second;
        }
        
        for (size_t j = 0; j < outSymbolsForState.size(); ++j) {
            new_transition += outSymbolsForState[j];
        }
        new_transition += "/";
        std::set<std::string> toStateNames;
        for (const auto& toState : nextStates) {
            toStateNames.insert(toState->getName());
        }

        for (auto it = toStateNames.begin(); it != toStateNames.end(); ) {
            new_transition += *it;
            ++it;
        }
        transitions_text.push_back(new_transition);
    }

    out["inputAlphabet"] = inputAlphabet_text;
    out["outputAlphabet"] = outputAlphabet_text;
    out["initialStates"] = initialStates_text;
    out["finalStates"] = finalStates_text;
    out["states"] = states_text;
    out["transitions"] = transitions_text;

    return out; 
}

// Getters
const Alphabet* Automaton::getAlphabet() const {
    return alphabet.get();
}

const Alphabet* Automaton::getOutAlphabet() const {
      return outAlphabet.get();
}


Alphabet* Automaton::getOutputAlphabet() const {
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

const std::vector<std::shared_ptr<State>>& Automaton::getInitialStates() const {
    return this->initialStates;
}

const std::vector<std::shared_ptr<State>>& Automaton::getStates() const {
    return states;
}

const std::vector<std::shared_ptr<State>>& Automaton::getFinalStates() const {
    return finalStates;
}

const std::unordered_map<std::pair<std::shared_ptr<State>, std::string>, std::vector<std::string>, PairHash
    >& Automaton::getOutSymbolsRelation() const {
    return outSymbolsRelation;
}

std::vector<std::string> Automaton::getOutputOfTransition(const std::shared_ptr<State> &current, const std::string &input) const {

    const std::pair<std::shared_ptr<State>, std::string> key = std::make_pair(current, input);
    auto it = outSymbolsRelation.find(key);
    if (it != outSymbolsRelation.end()) {
        return it->second;
    }
    return {}; // Return an empty vector if no such transition exists
}

const TransitionsRelation* Automaton::getTransitionsRelation() const {
    return transitionsRelation.get();
}
TransitionsRelation* Automaton::_getTransitionsRelation() const
{
    return transitionsRelation.get();
}

// Setters
void Automaton::setAlphabet(std::unique_ptr<Alphabet> newAlphabet) {
    alphabet = std::move(newAlphabet);
}

void Automaton::setInitialStates(const std::vector<std::shared_ptr<State>>& newInitialStates) {
    initialStates = newInitialStates;
}

void Automaton::setStates(const std::vector<std::shared_ptr<State>>& newStates) {
    states = newStates;
}

void Automaton::setFinalStates(const std::vector<std::shared_ptr<State>>& newFinalStates) {
    finalStates = newFinalStates;
}

void Automaton::setTransitionsRelation(std::unique_ptr<TransitionsRelation> newTransitionsRelation) {
    transitionsRelation = std::move(newTransitionsRelation);
}


/**
 * Returns state that comprises ONLY given elementary state. If automaton does not contain such state, empty pointer is returned
 */
std::shared_ptr<State> Automaton::findStateByElementaryState(const std::set<int>& elementaryState) const {
    for(const std::shared_ptr<State>& s : this->states) {
        if(s->isElementary() && elementaryState == s->getElementaryStates()[0]) {
            return s;
        }
    }
    return std::shared_ptr<State>{};
}

/**
 * Check if each transition has maximum 1 end state
 */
bool Automaton::isDeterministic() const {
    for (const auto& [key, nextStates] : this->transitionsRelation->getTransitions()) {
        if(nextStates.size() > 1) {
            return false;
        }
    }
    return true;
}
