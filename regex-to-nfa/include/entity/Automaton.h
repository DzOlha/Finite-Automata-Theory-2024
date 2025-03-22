#ifndef AUTOMATA_H
#define AUTOMATA_H

#include <memory>
#include <vector>
#include <entity/Alphabet.h>
#include <entity/State.h>
#include <entity/TransitionsRelation.h>

/**
 * Class that represents finite automaton
 */
class Automaton {
private:
    std::unique_ptr<Alphabet> alphabet;
    std::shared_ptr<State> initialState;
    std::vector<std::shared_ptr<State>> states;
    std::vector<std::shared_ptr<State>> finalStates;
    std::unique_ptr<TransitionsRelation> transitionsRelation;

public:
    Automaton();
    Automaton(std::unique_ptr<Alphabet> alphabet,
             std::shared_ptr<State> initialState,
             std::vector<std::shared_ptr<State>> states,
             std::vector<std::shared_ptr<State>> finalStates,
             TransitionsRelation::TransitionMap transitionMap);
    std::unique_ptr<Automaton> clone() const;

    void print() const;

    // Getters
    [[nodiscard]] const Alphabet* getAlphabet() const;
    Alphabet* _getAlphabet() const;
    [[nodiscard]] const State* getInitialState() const;
    [[nodiscard]] std::shared_ptr<State> getInitialStateShared() const;
    [[nodiscard]] const std::vector<std::shared_ptr<State>>& getStates() const;
    [[nodiscard]] const std::vector<std::shared_ptr<State>>& getFinalStates() const;
    [[nodiscard]] const TransitionsRelation* getTransitionsRelation() const;
    TransitionsRelation* _getTransitionsRelation() const;

    // Setters
    void setAlphabet(std::unique_ptr<Alphabet> newAlphabet);
    void setInitialState(std::shared_ptr<State> newInitialState);
    void setStates(const std::vector<std::shared_ptr<State>>& newStates);
    void setFinalStates(const std::vector<std::shared_ptr<State>>& newFinalStates);
    void setTransitionsRelation(std::unique_ptr<TransitionsRelation> newTransitionsRelation);

    [[nodiscard]] std::shared_ptr<State> findStateBySubstates(const std::set<int>& substates) const;
    [[nodiscard]] bool isDeterministic() const;

};

#endif // AUTOMATA_H
