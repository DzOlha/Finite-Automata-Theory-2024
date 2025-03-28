#include <entity/AutomatonBuilder.h>
#include <entity/Automaton.h>

// std::vector<std::set> is representation of State

// Utility function to get or create a state
std::shared_ptr<State> AutomatonBuilder::getState(const std::vector<std::set<int>>& stateId) {
    auto it = stateMap.find(stateId);
    if (it == stateMap.end()) { // create new State object
        auto state = std::make_shared<State>(stateId);
        stateMap[stateId] = state;
        return state;
    }
    return it->second;
}

AutomatonBuilder& AutomatonBuilder::setStates(const std::vector<std::vector<std::set<int>>>& stateIds) {
    states.clear();
    for (const auto& id : stateIds) {
        states.push_back(getState(id));
    }
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setInitialStates(const std::vector<std::vector<std::set<int>>>& initialStateIds) {
    initialStates.clear();
    for (const auto& id : initialStateIds) {
        initialStates.push_back(getState(id));
    }
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setFinalStates(const std::vector<std::vector<std::set<int>>>& finalStateIds) {
    finalStates.clear();
    for (const auto& id : finalStateIds) {
        finalStates.push_back(getState(id));
    }
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setAlphabet(const std::vector<std::string>& alpha) {
    alphabet = alpha;
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setOutAlphabet(const std::vector<std::string>& alpha) {
    outAlphabet = alpha;
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setTransitions(
    const std::vector<std::tuple<std::vector<std::set<int>>, // fromState
                                std::vector<std::string>, // input Symbols
                                std::vector<std::set<int>>, // toState
                                std::vector<std::string>>>& trans // outputSymbols
    ) {
    transitions.clear();
    for (const auto& [fromId, inputs, toId, outSymbols] : trans) {
        auto fromState = getState(fromId);
        auto toState = getState(toId);

        const size_t size = inputs.size();

        for (size_t i = 0; i < size; ++i) {
            const std::string& input = inputs[i];

            // Add transition for the input
            transitions[{fromState, input}].push_back(toState);

            // Associate output from outSymbols with the current input
            if(inputs.size() == 1)
            {
                for(const auto& out : outSymbols)
                {
                    outs[{fromState, input}].push_back(out);
                }
            } else {
                if (i < outSymbols.size()) { // Ensure that we don't go out of bounds
                    outs[{fromState, input}].push_back(outSymbols[i]);
                }
            }
        }
    }
    return *this;
}

Automaton AutomatonBuilder::build() {
    // Build the automaton with consistent state pointers
    return Automaton{
        std::make_unique<Alphabet>(alphabet),
        initialStates,  // Use std::vector<std::shared_ptr<State>>
        states,        // Use std::vector<std::shared_ptr<State>>
        finalStates,   // Use std::vector<std::shared_ptr<State>>
        std::move(transitions), // Use std::move if transitions is a unique_ptr
        outs,
        std::make_unique<Alphabet>(outAlphabet)
    };
}
