#include <entity/AutomatonBuilder.h>
#include <entity/Automaton.h>

// Utility function to get or create a state
std::shared_ptr<State> AutomatonBuilder::getState(int stateId) {
    auto it = stateMap.find(stateId);
    if (it == stateMap.end()) {
        auto state = std::make_shared<State>(std::set<int>{stateId});
        stateMap[stateId] = state;
        return state;
    }
    return it->second;
}

AutomatonBuilder& AutomatonBuilder::setInitialState(int stateId) {
    initialState = getState(stateId); // Use the getState function to ensure uniqueness
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setFinalStates(const std::vector<int>& finalStateIds) {
    finalStates.clear();
    for (int id : finalStateIds) {
        finalStates.push_back(getState(id));
    }
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setAlphabet(const std::vector<char>& alpha) {
    alphabet = alpha;
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setStates(const std::vector<int>& stateIds) {
    states.clear();
    for (int id : stateIds) {
        states.push_back(getState(id));
    }
    return *this;
}

AutomatonBuilder& AutomatonBuilder::setTransitions(const std::vector<std::tuple<int, std::vector<char>, int>>& trans) {
    transitions.clear();
    for (const auto& [fromId, inputs, toId] : trans) {
        auto fromState = getState(fromId);
        auto toState = getState(toId);
        for (char input : inputs) {
            transitions[{fromState, input}].push_back(toState);
        }
    }
    return *this;
}

Automaton AutomatonBuilder::build() {
    // Build the automaton with consistent state pointers
    return Automaton{
        std::make_unique<Alphabet>(alphabet),
        initialState,  // Use std::shared_ptr<State>
        states,        // Use std::vector<std::shared_ptr<State>>
        finalStates,   // Use std::vector<std::shared_ptr<State>>
        std::move(transitions) // Use std::move if transitions is a unique_ptr
    };
}
