#include <algo/RegexToMealy.h>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <map>
#include <stack>

// Constructor
RegexToMealy::RegexToMealy(const std::string& regex) : regex(regex), stateCounter(0)
{
}

std::pair<std::string, std::string> RegexToMealy::parsePair()
{
    if (regex.empty() || regex[0] != '[')
    {
        throw std::runtime_error("Expected '[' at the start of a pair");
    }
    regex = regex.substr(1);

    size_t slashPos = regex.find('/');
    if (slashPos == std::string::npos)
    {
        throw std::runtime_error("Expected '/' in a pair");
    }

    std::string input = regex.substr(0, slashPos);
    regex = regex.substr(slashPos + 1);

    size_t closePos = regex.find(']');
    if (closePos == std::string::npos)
    {
        throw std::runtime_error("Expected ']' at the end of a pair");
    }

    std::string output = regex.substr(0, closePos);
    regex = regex.substr(closePos + 1);

    //return '(' + input + ',' + output + ')';
    return {input, output};
}

std::unique_ptr<Automaton> RegexToMealy::parseExpression()
{
    auto result = parseTerm();
    while (!regex.empty() && regex[0] == '|')
    {
        regex = regex.substr(1);
        auto right = parseTerm();
        result = handleDisjunction(std::move(result), std::move(right));
    }
    return result;
}

std::unique_ptr<Automaton> RegexToMealy::parseTerm()
{
    auto result = parseFactor();
    while (!regex.empty() && regex[0] != ')' && regex[0] != '|')
    {
        auto right = parseFactor();
        result = handleConcatenation(std::move(result), std::move(right));
    }
    return result;
}

std::unique_ptr<Automaton> RegexToMealy::parseFactor()
{
    std::unique_ptr<Automaton> result;
    // if (regex.empty()) {
    //     return handleEmptyWord();
    // }

    if (regex.substr(0, 3) == "EPS")
    {
        regex = regex.substr(3);
        return handleEmptyWord();
    }

    char c = regex[0];

    if (c == '(')
    {
        regex = regex.substr(1);
        result = parseExpression();
        if (regex[0] == ')')
        {
            regex = regex.substr(1);
        }
        else
        {
            throw std::runtime_error("Mismatched parentheses");
        }
    }
    else
    {
        auto [input, output] = parsePair();
        result = handleElementaryLanguage(input, output);
    }

    while (!regex.empty() && regex[0] == '*')
    {
        result = handleKleeneStar(std::move(result));
        regex = regex.substr(1);
    }

    return result;
}

std::shared_ptr<State> RegexToMealy::createState(bool isFinal)
{
    std::set<int> substates{stateCounter++};
    return std::make_shared<State>(substates, isFinal);
}

void RegexToMealy::addTransition(TransitionsRelation::TransitionMap& transitions,
                                 const std::shared_ptr<State>& from,
                                 const std::string& symbol,
                                 const std::shared_ptr<State>& to,
                                 Automaton::OutputSymbolMap& outs,
                                 const std::string& out
)
{
    TransitionsRelation::TransitionKey key = std::make_pair(from, symbol);
    auto it = transitions.find(key);
    if (it != transitions.end())
    {
        if (std::find(it->second.begin(), it->second.end(), to) == it->second.end())
        {
            it->second.push_back(to);
        }
    }
    else
    {
        transitions.insert({key, {to}});
    }

    outs[key].push_back(out);
}

std::pair<std::unique_ptr<Alphabet>, std::unique_ptr<Alphabet>> RegexToMealy::mergeAlphabets(
    const std::unique_ptr<Automaton>& left, const std::unique_ptr<Automaton>& right
)
{
    auto newAlphabet = std::make_unique<Alphabet>(left->_getAlphabet()->_getLetters());
    auto& newAlphabetLetters = newAlphabet->_getLetters();
    const auto& rightAlphabetLetters = right->_getAlphabet()->_getLetters();
    for (const std::string& c : rightAlphabetLetters)
    {
        if (std::find(newAlphabetLetters.begin(), newAlphabetLetters.end(), c) == newAlphabetLetters.end())
        {
            newAlphabetLetters.push_back(c);
        }
    }

    auto newOutAlphabet = std::make_unique<Alphabet>(left->getOutputAlphabet()->_getLetters());
    auto& newOutAlphabetLetters = newOutAlphabet->_getLetters();
    const auto& rightOutAlphabetLetters = right->getOutputAlphabet()->_getLetters();
    for (const std::string& c : rightOutAlphabetLetters)
    {
        if (std::find(newOutAlphabetLetters.begin(), newOutAlphabetLetters.end(), c) == newOutAlphabetLetters.end())
        {
            newOutAlphabetLetters.push_back(c);
        }
    }

    return std::make_pair(std::move(newAlphabet), std::move(newOutAlphabet));
}

std::unique_ptr<Automaton> RegexToMealy::handleElementaryLanguage(
    const std::string& input, const std::string& output
)
{
    auto initialState = createState();
    auto finalState = createState(true);
    TransitionsRelation::TransitionMap transitions;
    Automaton::OutputSymbolMap outs;
    addTransition(transitions, initialState, input, finalState, outs, output);

    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(std::vector<std::string>{input}),
        initialState,
        std::vector<std::shared_ptr<State>>{initialState, finalState},
        std::vector<std::shared_ptr<State>>{finalState},
        transitions,
        outs,
        std::make_unique<Alphabet>(std::vector<std::string>{output})
    );
}

std::unique_ptr<Automaton> RegexToMealy::handleEmptyWord()
{
    auto state = createState(true);
    return std::make_unique<Automaton>(
        std::make_unique<Alphabet>(std::vector<std::string>{}),
        state,
        std::vector<std::shared_ptr<State>>{state},
        std::vector<std::shared_ptr<State>>{state},
        TransitionsRelation::TransitionMap(),
        Automaton::OutputSymbolMap(),
        std::make_unique<Alphabet>(std::vector<std::string>{})
    );
}

std::unique_ptr<Automaton> RegexToMealy::handleDisjunction(
    std::unique_ptr<Automaton> left, std::unique_ptr<Automaton> right
)
{
    auto newInitialState = createState();
    auto newFinalState = createState(true);

    auto newTransitions = left->getTransitionsRelation()->getTransitions();
    auto newOutFunction = left->getOutputFunction();

    auto rightOuts = right->getOutputFunction();

    // Add all transitions from right automata
    for (const auto& [key, value] : right->getTransitionsRelation()->getTransitions())
    {
        int counter = 0;
        for (const auto& toState : value)
        {
            addTransition(newTransitions, key.first, key.second, toState,
                          newOutFunction, rightOuts[key][counter]
            );
            counter++;
        }
    }

    // Connect new initial state to both left and right initial states
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), left->getInitialStateShared(),
                  newOutFunction, "");
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), right->getInitialStateShared(),
                  newOutFunction, "");

    // Connect all final states to the new final state
    for (const auto& finalState : left->getFinalStates())
    {
        addTransition(newTransitions, finalState, std::string(1, '\0'), newFinalState,
                      newOutFunction, "");
    }
    for (const auto& finalState : right->getFinalStates())
    {
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
        newInitialState,
        newStates,
        std::vector<std::shared_ptr<State>>{newFinalState},
        newTransitions,
        newOutFunction,
        std::move(outAlphabet)
    );
}

std::unique_ptr<Automaton> RegexToMealy::handleConcatenation(
    std::unique_ptr<Automaton> left, std::unique_ptr<Automaton> right
)
{
    auto newTransitions = left->getTransitionsRelation()->getTransitions();

    auto newOutFunction = left->getOutputFunction();
    auto rightOuts = right->getOutputFunction();

    // Add all transitions from right automata
    for (const auto& [key, value] : right->getTransitionsRelation()->getTransitions())
    {
        int counter = 0;
        for (const auto& toState : value)
        {
            addTransition(newTransitions, key.first, key.second, toState,
                          newOutFunction, rightOuts[key][counter]);
            counter++;
        }
    }

    // Connect final states of left to initial state of right
    for (const auto& finalState : left->getFinalStates())
    {
        addTransition(newTransitions, finalState, std::string(1, '\0'), right->getInitialStateShared(),
                      newOutFunction, "");
    }

    // Merge states and set new final states
    auto newStates = left->getStates();
    newStates.insert(newStates.end(), right->getStates().begin(), right->getStates().end());

    // Merge alphabets
    auto [inAlphabet, outAlphabet] = mergeAlphabets(left, right);


    return std::make_unique<Automaton>(
        std::move(inAlphabet),
        left->getInitialStateShared(),
        newStates,
        right->getFinalStates(),
        newTransitions,
        newOutFunction,
        std::move(outAlphabet)
    );
}

std::unique_ptr<Automaton> RegexToMealy::handleKleeneStar(std::unique_ptr<Automaton> automata)
{
    auto newInitialState = createState();
    auto newFinalState = createState(true);

    auto newTransitions = automata->getTransitionsRelation()->getTransitions();
    auto newOutFunction = automata->getOutputFunction();

    // Connect new initial state to new final state and old initial state
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), newFinalState,
                  newOutFunction, "");
    addTransition(newTransitions, newInitialState, std::string(1, '\0'), automata->getInitialStateShared(),
                  newOutFunction, "");

    // Connect old final states to new final state and old initial state
    for (const auto& finalState : automata->getFinalStates())
    {
        addTransition(newTransitions, finalState, std::string(1, '\0'), newFinalState,
                      newOutFunction, "");
        addTransition(newTransitions, finalState, std::string(1, '\0'), automata->getInitialStateShared(),
                      newOutFunction, "");
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
        newTransitions,
        newOutFunction,
        std::make_unique<Alphabet>(automata->getOutputAlphabet()->getLetters())
    );
}

std::set<std::shared_ptr<State>> RegexToMealy::findEpsilonClosure(
    const std::shared_ptr<State>& state,
    const TransitionsRelation::TransitionMap& transitions)
{
    std::set<std::shared_ptr<State>> closure;
    std::stack<std::shared_ptr<State>> stack;
    stack.push(state);

    while (!stack.empty())
    {
        auto currentState = stack.top();
        stack.pop();

        if (closure.find(currentState) != closure.end()) continue;
        closure.insert(currentState);

        for (const auto& [key, value] : transitions)
        {
            if (key.first == currentState && key.second == std::string(1, '\0'))
            {
                for (const auto& nextState : value)
                {
                    if (closure.find(nextState) == closure.end())
                    {
                        stack.push(nextState);
                    }
                }
            }
        }
    }

    return closure;
}

void RegexToMealy::buildNewTransitions(
    const TransitionsRelation::TransitionMap& oldTransitions,
    TransitionsRelation::TransitionMap& newTransitions,
    Automaton::OutputSymbolMap& oldOutFunction,
    Automaton::OutputSymbolMap& newOutFunction,
    const std::vector<std::shared_ptr<State>>& states)
{
    for (const auto& state : states)
    {
        auto closure = findEpsilonClosure(state, oldTransitions);

        for (const auto& closureState : closure)
        {
            for (const auto& [key, value] : oldTransitions)
            {
                int counter = 0;
                if (key.first == closureState && key.second != std::string(1, '\0'))
                {
                    for (const auto& reachableState : value)
                    {
                        addTransition(newTransitions, state, key.second, reachableState,
                                      newOutFunction, oldOutFunction[key][counter]);
                    }
                }
                counter++;
            }
        }
    }
}

std::set<std::shared_ptr<State>> RegexToMealy::findReachableStates(
    const std::shared_ptr<State>& initialState,
    const TransitionsRelation::TransitionMap& transitions)
{
    std::set<std::shared_ptr<State>> reachableStates;
    std::stack<std::shared_ptr<State>> stack;
    stack.push(initialState);

    while (!stack.empty())
    {
        auto state = stack.top();
        stack.pop();

        if (reachableStates.find(state) != reachableStates.end()) continue;
        reachableStates.insert(state);

        for (const auto& [key, value] : transitions)
        {
            if (key.first == state)
            {
                for (const auto& nextState : value)
                {
                    if (reachableStates.find(nextState) == reachableStates.end())
                    {
                        stack.push(nextState);
                    }
                }
            }
        }
    }

    return reachableStates;
}

// Remove epsilon transitions and compute direct character transitions
void RegexToMealy::removeEpsilonTransitions(std::unique_ptr<Automaton>& automata)
{
    // Get current transitions, states, initial and final states
    auto oldTransitions = automata->getTransitionsRelation()->getTransitions();
    auto oldOutFunction = automata->getOutputFunction();
    auto states = automata->getStates();
    auto initialState = automata->getInitialStateShared();
    auto finalStates = automata->getFinalStates();

    // New transition map after removing epsilon transitions
    TransitionsRelation::TransitionMap newTransitions;
    Automaton::OutputSymbolMap newOutFunction;

    // Build transitions for each state considering epsilon closures
    buildNewTransitions(oldTransitions, newTransitions, oldOutFunction, newOutFunction, states);

    // Find new final states by checking if any state in its epsilon closure is a final state
    std::set<std::shared_ptr<State>> newFinalStates;
    for (const auto& state : states)
    {
        auto closure = findEpsilonClosure(state, oldTransitions);
        for (const auto& closureState : closure)
        {
            if (std::find(finalStates.begin(), finalStates.end(), closureState) != finalStates.end())
            {
                newFinalStates.insert(state); // Current state becomes final if any closure state is final
                break;
            }
        }
    }

    // Find all reachable states from the initial state
    auto reachableStates = findReachableStates(initialState, newTransitions);

    // Filter out unreachable transitions and states
    TransitionsRelation::TransitionMap finalTransitions;
    for (const auto& [key, value] : newTransitions)
    {
        if (reachableStates.find(key.first) != reachableStates.end())
        {
            std::vector<std::shared_ptr<State>> reachableTargets;
            for (const auto& target : value)
            {
                if (reachableStates.find(target) != reachableStates.end())
                {
                    reachableTargets.push_back(target); // Add only reachable target states
                }
            }
            if (!reachableTargets.empty())
            {
                finalTransitions[key] = reachableTargets; // Update final transitions map
            }
        }
    }

    // Collect reachable final states
    std::vector<std::shared_ptr<State>> finalStatesList;
    for (const auto& state : newFinalStates)
    {
        if (reachableStates.find(state) != reachableStates.end())
        {
            finalStatesList.push_back(state); // Only include reachable final states
        }
    }

    // Update the automata with new transitions, states, and final states
    automata->_getTransitionsRelation()->setTransitions(finalTransitions);
    automata->setStates(std::vector<std::shared_ptr<State>>(reachableStates.begin(), reachableStates.end()));
    automata->setFinalStates(finalStatesList);
    automata->setOutputFunction(newOutFunction);
}

std::unique_ptr<Automaton> RegexToMealy::convert()
{
    std::unique_ptr<Automaton> nfa = parseExpression();

    removeEpsilonTransitions(nfa);

    return nfa;
}
