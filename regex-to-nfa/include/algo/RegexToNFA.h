#ifndef REGEXTONFA_H
#define REGEXTONFA_H

#include <string>
#include <memory>
#include <set>
#include <vector>
#include <entity/Automaton.h>
#include <entity/State.h>
#include <entity/TransitionsRelation.h>

class RegexToNFA
{
private:
    std::string regex;
    int stateCounter;

    std::unique_ptr<Automaton> parseExpression();
    std::unique_ptr<Automaton> parseTerm();
    std::unique_ptr<Automaton> parseFactor();

    std::shared_ptr<State> createState(bool isFinal = false);
    static void addTransition(TransitionsRelation::TransitionMap& transitions,
                              const std::shared_ptr<State>& from, char symbol, const std::shared_ptr<State>& to);

    std::unique_ptr<Automaton> handleElementaryLanguage(char c);
    std::unique_ptr<Automaton> handleEmptyWord();
    std::unique_ptr<Automaton> handleDisjunction(std::unique_ptr<Automaton> left, std::unique_ptr<Automaton> right);
    std::unique_ptr<Automaton> handleConcatenation(std::unique_ptr<Automaton> left, std::unique_ptr<Automaton> right);
    std::unique_ptr<Automaton> handleKleeneStar(std::unique_ptr<Automaton> automata);

    static std::set<std::shared_ptr<State>> findEpsilonClosure(
        const std::shared_ptr<State>& state,
        const TransitionsRelation::TransitionMap& transitions);
    static void buildNewTransitions(
        const TransitionsRelation::TransitionMap& oldTransitions,
        TransitionsRelation::TransitionMap& newTransitions,
        const std::vector<std::shared_ptr<State>>& states);
    static std::set<std::shared_ptr<State>> findReachableStates(
        const std::shared_ptr<State>& initialState,
        const TransitionsRelation::TransitionMap& transitions);

public:
    explicit RegexToNFA(const std::string& regex);

    // Remove epsilon transitions and compute direct character transitions
    static void removeEpsilonTransitions(std::unique_ptr<Automaton>& automata);

    std::unique_ptr<Automaton> convert();
};

#endif // REGEXTONFA_H
