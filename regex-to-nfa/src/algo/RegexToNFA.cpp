#include <algo/RegexToNFA.h>
#include <stdexcept>
#include <algorithm>
#include <stack>

// Constructor
RegexToNFA::RegexToNFA(const std::string& regex) : regex(regex), stateCounter(0) {}

std::unique_ptr<Automaton> RegexToNFA::parseExpression() {
    auto result = parseTerm();
    while (regex.length() > 0 && regex[0] == '|') {
        regex = regex.substr(1);
        auto right = parseTerm();
        result = handleDisjunction(std::move(result), std::move(right));
    }
    return result;
}

std::unique_ptr<Automaton> RegexToNFA::parseTerm() {
    auto result = parseFactor();
    while (regex.length() > 0 && regex[0] != ')' && regex[0] != '|') {
        auto right = parseFactor();
        result = handleConcatenation(std::move(result), std::move(right));
    }
    return result;
}

std::unique_ptr<Automaton> RegexToNFA::parseFactor() {
    std::unique_ptr<Automaton> result;
    if (regex.empty()) {
        return handleEmptyWord();
    }

    if (regex.substr(0, 3) == "EPS") {
        regex = regex.substr(3);
        return handleEmptyWord();
    }

    char c = regex[0];
    regex = regex.substr(1);

    if (c == '(') {
        result = parseExpression();
        if (regex[0] == ')') {
            regex = regex.substr(1);
        } else {
            throw std::runtime_error("Mismatched parentheses");
        }
    } else {
        result = handleElementaryLanguage(c);
    }

    while (regex.length() > 0 && regex[0] == '*') {
        result = handleKleeneStar(std::move(result));
        regex = regex.substr(1);
    }

    return result;
}


std::shared_ptr<State> RegexToNFA::createState(bool isFinal) {
    std::set<int> substates{stateCounter++};
    return std::make_shared<State>(substates, isFinal);
}

void RegexToNFA::addTransition(TransitionsRelation::TransitionMap& transitions,
                          const std::shared_ptr<State>& from, char symbol, const std::shared_ptr<State>& to) {
    TransitionsRelation::TransitionKey key = std::make_pair(from, symbol);
    auto it = transitions.find(key);
    if (it != transitions.end()) {
        if (std::find(it->second.begin(), it->second.end(), to) == it->second.end()) {
            it->second.push_back(to);
        }
    } else {
        transitions.insert({key, {to}});
    }
}

std::unique_ptr<Automaton> RegexToNFA::handleElementaryLanguage(char c) {
    auto initialState = createState();
    auto finalState = createState(true);
    TransitionsRelation::TransitionMap transitions;
    addTransition(transitions, initialState, c, finalState);

    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(std::vector<char>{c}),
        initialState,
        std::vector<std::shared_ptr<State>>{initialState, finalState},
        std::vector<std::shared_ptr<State>>{finalState},
        transitions
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleEmptyWord() {
    auto state = createState(true);
    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(std::vector<char>{}),
        state,
        std::vector<std::shared_ptr<State>>{state},
        std::vector<std::shared_ptr<State>>{state},
        TransitionsRelation::TransitionMap()
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleDisjunction(std::unique_ptr<Automaton> left, std::unique_ptr<Automaton> right) {
    auto newInitialState = createState();
    auto newFinalState = createState(true);

    auto newTransitions = left->getTransitionsRelation()->getTransitions();

    // Add all transitions from right automata
    for (const auto& [key, value] : right->getTransitionsRelation()->getTransitions()) {
        for (const auto& toState : value) {
            addTransition(newTransitions, key.first, key.second, toState);
        }
    }

    // Connect new initial state to both left and right initial states
    addTransition(newTransitions, newInitialState, '\0', left->getInitialStateShared());
    addTransition(newTransitions, newInitialState, '\0', right->getInitialStateShared());

    // Connect all final states to the new final state
    for (const auto& finalState : left->getFinalStates()) {
        addTransition(newTransitions, finalState, '\0', newFinalState);
    }
    for (const auto& finalState : right->getFinalStates()) {
        addTransition(newTransitions, finalState, '\0', newFinalState);
    }

    // Merge states
    auto newStates = left->getStates();
    newStates.insert(newStates.end(), right->getStates().begin(), right->getStates().end());
    newStates.push_back(newInitialState);
    newStates.push_back(newFinalState);

    // Merge alphabets
    auto newAlphabet = std::make_unique<Alphabet>(left->_getAlphabet()->_getLetters());
    auto& newAlphabetLetters = newAlphabet->_getLetters();
    const auto& rightAlphabetLetters = right->_getAlphabet()->_getLetters();
    for (char c : rightAlphabetLetters) {
        if (std::find(newAlphabetLetters.begin(), newAlphabetLetters.end(), c) == newAlphabetLetters.end()) {
            newAlphabetLetters.push_back(c);
        }
    }

    return std::make_unique<Automaton>(
        std::move(newAlphabet),
        newInitialState,
        newStates,
        std::vector<std::shared_ptr<State>>{newFinalState},
        newTransitions
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleConcatenation(std::unique_ptr<Automaton> left, std::unique_ptr<Automaton> right) {
    auto newTransitions = left->getTransitionsRelation()->getTransitions();

    // Add all transitions from right automata
    for (const auto& [key, value] : right->getTransitionsRelation()->getTransitions()) {
        for (const auto& toState : value) {
            addTransition(newTransitions, key.first, key.second, toState);
        }
    }

    // Connect final states of left to initial state of right
    for (const auto& finalState : left->getFinalStates()) {
        addTransition(newTransitions, finalState, '\0', right->getInitialStateShared());
    }

    // Merge states and set new final states
    auto newStates = left->getStates();
    newStates.insert(newStates.end(), right->getStates().begin(), right->getStates().end());

    // Merge alphabets
    auto newAlphabet = std::make_unique<Alphabet>(left->_getAlphabet()->_getLetters());
    const auto& rightAlphabetLetters = right->_getAlphabet()->_getLetters();
    for (const char c : rightAlphabetLetters) {
        if (std::find(newAlphabet->_getLetters().begin(), newAlphabet->_getLetters().end(), c) == newAlphabet->_getLetters().end()) {
            newAlphabet->_getLetters().push_back(c);
        }
    }

    return std::make_unique<Automaton>(
        std::move(newAlphabet),
        left->getInitialStateShared(),
        newStates,
        right->getFinalStates(),
        newTransitions
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleKleeneStar(std::unique_ptr<Automaton> automata) {
    auto newInitialState = createState();
    auto newFinalState = createState(true);

    auto newTransitions = automata->getTransitionsRelation()->getTransitions();

    // Connect new initial state to new final state and old initial state
    addTransition(newTransitions, newInitialState, '\0', newFinalState);
    addTransition(newTransitions, newInitialState, '\0', automata->getInitialStateShared());

    // Connect old final states to new final state and old initial state
    for (const auto& finalState : automata->getFinalStates()) {
        addTransition(newTransitions, finalState, '\0', newFinalState);
        addTransition(newTransitions, finalState, '\0', automata->getInitialStateShared());
    }

    // Add new states
    auto newStates = automata->getStates();
    newStates.push_back(newInitialState);
    newStates.push_back(newFinalState);

    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(automata->_getAlphabet()->_getLetters()),
        newInitialState,
        newStates,
        std::vector<std::shared_ptr<State>>{newFinalState},
        newTransitions
    );
}

std::set<std::shared_ptr<State>> RegexToNFA::findEpsilonClosure(
    const std::shared_ptr<State>& state,
    const TransitionsRelation::TransitionMap& transitions) {
    std::set<std::shared_ptr<State>> closure;
    std::stack<std::shared_ptr<State>> stack;
    stack.push(state);

    while (!stack.empty()) {
        auto currentState = stack.top();
        stack.pop();

        if (closure.find(currentState) != closure.end()) continue;
        closure.insert(currentState);

        for (const auto& [key, value] : transitions) {
            if (key.first == currentState && key.second == '\0') {
                for (const auto& nextState : value) {
                    if (closure.find(nextState) == closure.end()) {
                        stack.push(nextState);
                    }
                }
            }
        }
    }

    return closure;
}

void RegexToNFA::buildNewTransitions(
    const TransitionsRelation::TransitionMap& oldTransitions,
    TransitionsRelation::TransitionMap& newTransitions,
    const std::vector<std::shared_ptr<State>>& states) {
    for (const auto& state : states) {
        auto closure = findEpsilonClosure(state, oldTransitions);

        for (const auto& closureState : closure) {
            for (const auto& [key, value] : oldTransitions) {
                if (key.first == closureState && key.second != '\0') {
                    for (const auto& reachableState : value) {
                        addTransition(newTransitions, state, key.second, reachableState);
                    }
                }
            }
        }
    }
}

std::set<std::shared_ptr<State>> RegexToNFA::findReachableStates(
    const std::shared_ptr<State>& initialState,
    const TransitionsRelation::TransitionMap& transitions) {
    std::set<std::shared_ptr<State>> reachableStates;
    std::stack<std::shared_ptr<State>> stack;
    stack.push(initialState);

    while (!stack.empty()) {
        auto state = stack.top();
        stack.pop();

        if (reachableStates.find(state) != reachableStates.end()) continue;
        reachableStates.insert(state);

        for (const auto& [key, value] : transitions) {
            if (key.first == state) {
                for (const auto& nextState : value) {
                    if (reachableStates.find(nextState) == reachableStates.end()) {
                        stack.push(nextState);
                    }
                }
            }
        }
    }

    return reachableStates;
}

// Remove epsilon transitions and compute direct character transitions
void RegexToNFA::removeEpsilonTransitions(std::unique_ptr<Automaton>& automata)
{
    // Get current transitions, states, initial and final states
    auto oldTransitions = automata->getTransitionsRelation()->getTransitions();
    auto states = automata->getStates();
    auto initialState = automata->getInitialStateShared();
    auto finalStates = automata->getFinalStates();

    // New transition map after removing epsilon transitions
    TransitionsRelation::TransitionMap newTransitions;

    // Build transitions for each state considering epsilon closures
    buildNewTransitions(oldTransitions, newTransitions, states);

    // Find new final states by checking if any state in its epsilon closure is a final state
    std::set<std::shared_ptr<State>> newFinalStates;
    for (const auto& state : states) {
        auto closure = findEpsilonClosure(state, oldTransitions);
        for (const auto& closureState : closure) {
            if (std::find(finalStates.begin(), finalStates.end(), closureState) != finalStates.end()) {
                newFinalStates.insert(state); // Current state becomes final if any closure state is final
                break;
            }
        }
    }

    // Find all reachable states from the initial state
    auto reachableStates = findReachableStates(initialState, newTransitions);

    // Filter out unreachable transitions and states
    TransitionsRelation::TransitionMap finalTransitions;
    for (const auto& [key, value] : newTransitions) {
        if (reachableStates.find(key.first) != reachableStates.end()) {
            std::vector<std::shared_ptr<State>> reachableTargets;
            for (const auto& target : value) {
                if (reachableStates.find(target) != reachableStates.end()) {
                    reachableTargets.push_back(target); // Add only reachable target states
                }
            }
            if (!reachableTargets.empty()) {
                finalTransitions[key] = reachableTargets; // Update final transitions map
            }
        }
    }

    // Collect reachable final states
    std::vector<std::shared_ptr<State>> finalStatesList;
    for (const auto& state : newFinalStates) {
        if (reachableStates.find(state) != reachableStates.end()) {
            finalStatesList.push_back(state); // Only include reachable final states
        }
    }

    // Update the automata with new transitions, states, and final states
    automata->_getTransitionsRelation()->setTransitions(finalTransitions);
    automata->setStates(std::vector<std::shared_ptr<State>>(reachableStates.begin(), reachableStates.end()));
    automata->setFinalStates(finalStatesList);
}


std::unique_ptr<Automaton> RegexToNFA::convert() {
    return parseExpression();
}