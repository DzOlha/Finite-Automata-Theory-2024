//
// Created by dzuha on 02/11/2024.
//
#include <algo/RegexToNFA.h>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <stack>

// Constructor
RegexToNFA::RegexToNFA(const std::string& regex) : regex(regex), stateCounter(0) {}

void RegexToNFA::setStateCounter(int newCounter)
{
    stateCounter = newCounter;
}

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
        std::string c_str(1, c);
        result = handleElementaryLanguage(c_str, "");
    }

    // iteration processing
    while (regex.length() > 0 && regex[0] == '*') {
        result = handleKleeneStar(std::move(result));
        regex = regex.substr(1);
    }

    // strong iteration processing
    while (regex.length() > 0 && regex[0] == '^') {
        result = handleKleeneStar(std::move(result));
        result = handleStrongIteration(std::move(result));
        regex = regex.substr(1);
    }

    return result;
}


std::shared_ptr<State> RegexToNFA::createState(bool isFinal) {
    std::set<int> substates{stateCounter++};
    std::vector states = {substates};
    return std::make_shared<State>(states, isFinal);
}

void RegexToNFA::addTransition(TransitionsRelation::TransitionMap& transitions,
                          const std::shared_ptr<State>& from,
                          const std::string& symbol,
                          const std::shared_ptr<State>& to,
                          Automaton::OutputSymbolMap& outs,
                          const std::string& out
) {
    TransitionsRelation::TransitionKey key = std::make_pair(from, symbol);
    auto it = transitions.find(key);
    if (it != transitions.end()) {
        if (std::find(it->second.begin(), it->second.end(), to) == it->second.end()) {
            it->second.push_back(to);
        }
    } else {
        transitions.insert({key, {to}});
    }

    outs[key].push_back(out);
}

std::pair<std::unique_ptr<Alphabet>, std::unique_ptr<Alphabet>> RegexToNFA::mergeAlphabets(
    const std::unique_ptr<Automaton>& left, const std::unique_ptr<Automaton>& right
){
    auto newAlphabet = std::make_unique<Alphabet>(left->_getAlphabet()->_getLetters());
    auto& newAlphabetLetters = newAlphabet->_getLetters();
    const auto& rightAlphabetLetters = right->_getAlphabet()->_getLetters();
    for (const std::string& c : rightAlphabetLetters) {
        if (std::find(newAlphabetLetters.begin(), newAlphabetLetters.end(), c) == newAlphabetLetters.end()) {
            newAlphabetLetters.push_back(c);
        }
    }

    auto newOutAlphabet = std::make_unique<Alphabet>(left->getOutputAlphabet()->_getLetters());
    auto& newOutAlphabetLetters = newOutAlphabet->_getLetters();
    const auto& rightOutAlphabetLetters = right->getOutputAlphabet()->_getLetters();
    for (const std::string& c : rightOutAlphabetLetters) {
        if (std::find(newOutAlphabetLetters.begin(), newOutAlphabetLetters.end(), c) == newOutAlphabetLetters.end()) {
            newOutAlphabetLetters.push_back(c);
        }
    }

    return std::make_pair(std::move(newAlphabet), std::move(newOutAlphabet));
}

std::unique_ptr<Automaton> RegexToNFA::handleElementaryLanguage(
    const std::string& input, const std::string& output
) {
    auto initialState = createState();
    auto finalState = createState(true);
    TransitionsRelation::TransitionMap transitions;
    Automaton::OutputSymbolMap outs;
    addTransition(transitions, initialState, input, finalState, outs, output);

    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(std::vector<std::string>{input}),
        std::vector<std::shared_ptr<State>>{initialState},
        std::vector<std::shared_ptr<State>>{initialState, finalState},
        std::vector<std::shared_ptr<State>>{finalState},
        transitions,
        outs,
        std::make_unique<Alphabet>(std::vector<std::string>{output})
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleEmptyWord() {
    auto state = createState(true);
    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(std::vector<std::string>{}),
        std::vector<std::shared_ptr<State>>{state},
        std::vector<std::shared_ptr<State>>{state},
        std::vector<std::shared_ptr<State>>{state},
        TransitionsRelation::TransitionMap(),
        Automaton::OutputSymbolMap(),
        std::make_unique<Alphabet>(std::vector<std::string>{})
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleDisjunction(
    std::unique_ptr<Automaton> left, std::unique_ptr<Automaton> right
) {
    auto newInitialState = createState();
    auto newFinalState = createState(true);

    auto newTransitions = left->getTransitionsRelation()->getTransitions();
    auto newOutFunction = left->getOutputFunction();

    auto rightOuts = right->getOutputFunction();

    // Add all transitions from right automata
    for (const auto& [key, value] : right->getTransitionsRelation()->getTransitions()) {
        int counter = 0;
        for (const auto& toState : value) {
            // Check if rightOuts contains the key and if the counter index is valid
            std::string outputSymbol = std::string(1, '\0');
            if (rightOuts.find(key) != rightOuts.end() && counter < rightOuts[key].size()) {
                outputSymbol = rightOuts[key][counter];
            }
            // Add the transition, using the output symbol or an empty string
            addTransition(newTransitions, key.first, key.second, toState,
                          newOutFunction, outputSymbol);
            counter++;
        }
    }

    // Connect new initial state to both left and right initial states
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), left->getInitialStates()[0],
        newOutFunction, "");
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), right->getInitialStates()[0],
        newOutFunction, "");

    // Connect all final states to the new final state
    for (const auto& finalState : left->getFinalStates()) {
        addTransition(newTransitions, finalState, std::string(1, '\0'), newFinalState,
            newOutFunction, "");
    }
    for (const auto& finalState : right->getFinalStates()) {
        addTransition(newTransitions, finalState, std::string(1, '\0'), newFinalState,
            newOutFunction, "");
    }

    // Merge states
    auto newStates = left->getStates();
    newStates.insert(newStates.end(), right->getStates().begin(), right->getStates().end());
    newStates.push_back(newInitialState);
    newStates.push_back(newFinalState);

    // Merge alphabets
    auto [inAlphabet, outAlphabet] = mergeAlphabets(left, right);

    return std::make_unique<Automaton>(
        std::move(inAlphabet),
        std::vector<std::shared_ptr<State>>{newInitialState},
        newStates,
        std::vector<std::shared_ptr<State>>{newFinalState},
        newTransitions,
        newOutFunction,
        std::move(outAlphabet)
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleConcatenation(
    std::unique_ptr<Automaton> left, std::unique_ptr<Automaton> right
    ) {
    auto newTransitions = left->getTransitionsRelation()->getTransitions();

    auto newOutFunction = left->getOutputFunction();
    auto rightOuts = right->getOutputFunction();

    // Add all transitions from right automata
    for (const auto& [key, value] : right->getTransitionsRelation()->getTransitions()) {
        int counter = 0;
        for (const auto& toState : value) {
            // Check if rightOuts contains the key and if the counter index is valid
            std::string outputSymbol = "";
            if (rightOuts.find(key) != rightOuts.end() && counter < rightOuts[key].size()) {
                outputSymbol = rightOuts[key][counter];
            }
            // Add the transition, using the output symbol or an empty string
            addTransition(newTransitions, key.first, key.second, toState,
                          newOutFunction, outputSymbol);
            counter++;
        }
    }

    // Connect final states of left to initial state of right
    for (const auto& finalState : left->getFinalStates()) {
        addTransition(newTransitions, finalState, std::string(1, '\0'), right->getInitialStates()[0],
            newOutFunction, "");
    }

    // Merge states and set new final states
    auto newStates = left->getStates();
    newStates.insert(newStates.end(), right->getStates().begin(), right->getStates().end());

    // Merge alphabets
    auto [inAlphabet, outAlphabet] = mergeAlphabets(left, right);


    return std::make_unique<Automaton>(
        std::move(inAlphabet),
        left->getInitialStates(),
        newStates,
        right->getFinalStates(),
        newTransitions,
        newOutFunction,
        std::move(outAlphabet)
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleKleeneStar(std::unique_ptr<Automaton> automata) {
    auto newInitialState = createState();
    auto newFinalState = createState(true);

    auto newTransitions = automata->getTransitionsRelation()->getTransitions();
    auto newOutFunction = automata->getOutputFunction();

    // Connect new initial state to new final state and old initial state
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), newFinalState,
        newOutFunction, "");
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), automata->getInitialStates()[0],
        newOutFunction, "");

    // Connect old final states to new final state and old initial state
    for (const auto& finalState : automata->getFinalStates()) {
        addTransition(newTransitions, finalState, std::string(1, '\0'), newFinalState,
            newOutFunction, "");
        addTransition(newTransitions, finalState, std::string(1, '\0'), automata->getInitialStates()[0],
            newOutFunction, "");
    }

    // Add new states
    auto newStates = automata->getStates();
    newStates.push_back(newInitialState);
    newStates.push_back(newFinalState);

    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(automata->_getAlphabet()->_getLetters()),
        std::vector<std::shared_ptr<State>>{newInitialState},
        newStates,
        std::vector<std::shared_ptr<State>>{newFinalState},
        newTransitions,
        newOutFunction,
        std::make_unique<Alphabet>(automata->getOutputAlphabet()->getLetters())
    );
}

std::unique_ptr<Automaton> RegexToNFA::handleStrongIteration(std::unique_ptr<Automaton> automata)
{
    // Create a new initial state (this state will also act as an entry point for looping)
    auto newInitialState = createState(true);

    // Retrieve the original automaton's transitions and output function
    auto newTransitions = automata->getTransitionsRelation()->getTransitions();
    auto newOutFunction = automata->getOutputFunction();

    // Connect the new initial state to the original initial state using an epsilon transition
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), automata->getInitialStates()[0],
        newOutFunction, "");

    // Loop from each original final state back to the original initial state
    for (const auto& finalState : automata->getFinalStates()) {
        addTransition(newTransitions, finalState, std::string(1, '\0'), newInitialState,
            newOutFunction, "");
    }

    // Also add a self-loop on the new initial state to handle cases where the repetition starts over
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), newInitialState,
        newOutFunction, "");

    // Collect all states from the original automaton and add the new initial state
    auto newStates = automata->getStates();
    auto newFinalStates = automata->getFinalStates();
    newStates.push_back(newInitialState);
    // newFinalStates.push_back(newInitialState);

    // Return a new automaton with the modified structure
    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(automata->_getAlphabet()->_getLetters()),
        std::vector<std::shared_ptr<State>>{newInitialState},
        newStates,
        newFinalStates, // Use the original final states as markers
        newTransitions,
        newOutFunction,
        std::make_unique<Alphabet>(automata->getOutputAlphabet()->getLetters())
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
            if (key.first == currentState && key.second == std::string(1, '\0')) {
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
    Automaton::OutputSymbolMap& oldOutFunction,
    Automaton::OutputSymbolMap& newOutFunction,
    const std::vector<std::shared_ptr<State>>& states)
{
    for (const auto& state : states) {
        auto closure = findEpsilonClosure(state, oldTransitions);

        for (const auto& closureState : closure) {
            for (const auto& [key, value] : oldTransitions) {
                int counter = 0;
                if (key.first == closureState && key.second != std::string(1, '\0')) {
                    for (const auto& reachableState : value) {
                        addTransition(newTransitions, state, key.second, reachableState,
                            newOutFunction, oldOutFunction[key][counter]);
                    }
                }
                counter++;
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
    auto oldOutFunction = automata->getOutputFunction();
    auto states = automata->getStates();
    auto initialState = automata->getInitialStates()[0];
    auto finalStates = automata->getFinalStates();

    // New transition map after removing epsilon transitions
    TransitionsRelation::TransitionMap newTransitions;
    Automaton::OutputSymbolMap newOutFunction;

    // Build transitions for each state considering epsilon closures
    buildNewTransitions(oldTransitions, newTransitions, oldOutFunction, newOutFunction, states);

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
    automata->setOutputFunction(newOutFunction);
}

bool RegexToNFA::hasState(std::vector<std::shared_ptr<State>> states, std::shared_ptr<State> state)
{
    return std::find(states.begin(), states.end(), state) != states.end();
}

void RegexToNFA::makeTwoPoleAutomaton(std::unique_ptr<Automaton>& automaton) {
    // Ensure there are initial and final states to process
    if (automaton->getInitialStates().empty() || automaton->getFinalStates().empty()) {
        return; // Nothing to do
    }

    // Try to select a state that is both initial and final
    auto initialStates = automaton->getInitialStates();
    auto finalStates = automaton->getFinalStates();
    std::shared_ptr<State> selectedInitialState = nullptr;
    std::shared_ptr<State> selectedFinalState = nullptr;

    for (const auto& initialState : initialStates) {
        if (this->hasState(finalStates, initialState)) {
            // Found a state that is both initial and final
            selectedInitialState = initialState;
            selectedFinalState = initialState;
            break;
        }
    }

    // If no common state is found, default to the first initial and final states
    if (!selectedInitialState || !selectedFinalState) {
        selectedInitialState = initialStates[0];
        selectedFinalState = finalStates[0];
    }

    // Get the current transitions
    auto existingTransitions = automaton->getTransitionsRelation()->getTransitions();
    TransitionsRelation::TransitionMap newTransitions;
    Automaton::OutputSymbolMap newOutFunction;

    // Loop through all initial states
    for (const auto& initialState : automaton->getInitialStates()) {
        if (initialState != selectedInitialState) {
            // Redirect transitions from the current initial state
            for (const auto& transition : existingTransitions) {
                auto fromState = transition.first.first;
                auto symbol = transition.first.second;
                const auto& toStates = transition.second;

                // If the transition originates from the current initial state
                if (fromState == initialState) {
                    for (const auto& toState : toStates) {
                        // Add a new transition from the selected initial state
                        addTransition(
                            newTransitions, selectedInitialState,
                            symbol, toState, newOutFunction, ""
                            );
                    }
                }

                // If the transition leads to the current initial state
                for (const auto& toState : toStates) {
                    if (toState == initialState) {
                        // Redirect to the selected initial state
                        addTransition(
                            newTransitions, fromState, symbol,
                            selectedInitialState, newOutFunction, ""
                            );
                    }
                }
            }
        }
    }

    // Loop through all final states
    for (const auto& finalState : automaton->getFinalStates()) {
        if (finalState != selectedFinalState) {
            // Redirect transitions to the current final state
            for (const auto& transition : existingTransitions) {
                auto fromState = transition.first.first;
                auto symbol = transition.first.second;
                const auto& toStates = transition.second;

                // If the transition leads to the current final state
                for (const auto& toState : toStates) {
                    if (toState == finalState) {
                        // Add a new transition to the selected final state
                        addTransition(
                            newTransitions, fromState, symbol,
                            selectedFinalState, newOutFunction, ""
                            );
                    }
                }

                // If the transition originates from the current final state
                if (fromState == finalState) {
                    for (const auto& toState : toStates) {
                        // Redirect from the selected final state
                        addTransition(
                            newTransitions, selectedFinalState,
                            symbol, toState, newOutFunction, ""
                            );
                    }
                }
            }
        }
    }

    // Copy over transitions that don't involve excess initial or final states
    for (const auto& transition : existingTransitions) {
        auto fromState = transition.first.first;
        auto symbol = transition.first.second;
        const auto& toStates = transition.second;

        for (const auto& toState : toStates) {
            // Add existing transitions that are unrelated to excess states
            addTransition(
                newTransitions, fromState, symbol,
                toState, newOutFunction, ""
                );
        }
    }

    // Update the automaton with the new transitions
    automaton->_getTransitionsRelation()->setTransitions(newTransitions);

    // Set only the selected initial and final states
    automaton->setInitialStates({ selectedInitialState });
    automaton->setFinalStates({ selectedFinalState });
}


void RegexToNFA::makeOmegaAutomaton(std::unique_ptr<Automaton>& automaton)
{
    automaton = this->handleKleeneStar(std::move(automaton));
    automaton = this->handleStrongIteration(std::move(automaton));

    removeEpsilonTransitions(automaton);

    // makeTwoPoleAutomaton(automaton);
}

/**
 * This function takes two automata as arguments, which are built by
 * the convertToNFA function of the current class for the given languages regexes
 * @param first
 * @param second
 * @return
 */
std::unique_ptr<Automaton> RegexToNFA::makeConcatenatedAutomaton(
        std::unique_ptr<Automaton>& first, std::unique_ptr<Automaton>& second
){
    // make two pole automata
    makeTwoPoleAutomaton(first);
    makeTwoPoleAutomaton(second);

    // concatenate the given automata
    this->setStateCounter(first->getStates().size() + second->getStates().size());
    auto res = this->handleConcatenation(std::move(first), std::move(second));

    removeEpsilonTransitions(res);

    return res;
}

std::unique_ptr<Automaton> RegexToNFA::convertToNFA() {
    std::unique_ptr<Automaton> nfa = parseExpression();

    removeEpsilonTransitions(nfa);

    return nfa;
}

