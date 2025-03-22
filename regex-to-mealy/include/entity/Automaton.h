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
class Automaton
{
public:
    // Define a type alias for the output symbols map
    using OutputSymbolMap = std::unordered_map<
        std::pair<std::shared_ptr<State>, std::string>,
        std::vector<std::string>,
        PairHash
    >;
    Automaton();
    Automaton(std::unique_ptr<Alphabet> alphabet,
              std::shared_ptr<State> initialState,
              std::vector<std::shared_ptr<State>> states,
              std::vector<std::shared_ptr<State>> finalStates,
              TransitionsRelation::TransitionMap transitionMap,
              const OutputSymbolMap& outSymbolsMap = {},
              std::unique_ptr<Alphabet> outAlphabet = nullptr
    );

    std::vector<std::string> getAllOutputsByInput(const std::string& input, bool reverse = false);

    [[nodiscard]] const Alphabet* getOutAlphabet() const;

    std::unique_ptr<Automaton> clone() const;

    void print() const;
    void printMoore() const;
    // Getters
    [[nodiscard]] const Alphabet* getAlphabet() const;

    Alphabet* getOutputAlphabet() const;
    OutputSymbolMap getOutputFunction() const;
    void setOutputFunction(const OutputSymbolMap& out);

    Alphabet* _getAlphabet() const;
    [[nodiscard]] const State* getInitialState() const;
    [[nodiscard]] std::shared_ptr<State> getInitialStateShared() const;
    [[nodiscard]] const std::vector<std::shared_ptr<State>>& getStates() const;
    [[nodiscard]] const std::vector<std::shared_ptr<State>>& getFinalStates() const;
    [[nodiscard]] const TransitionsRelation* getTransitionsRelation() const;

    [[nodiscard]] const std::unordered_map<std::string, std::vector<std::string>> getAutomatonInText() const;


    [[nodiscard]] const std::unordered_map<std::pair<std::shared_ptr<State>, std::string>, std::vector<std::string>,
                                           PairHash>&
    getOutSymbolsRelation() const;

    TransitionsRelation* _getTransitionsRelation() const;

    // Setters
    void setAlphabet(std::unique_ptr<Alphabet> newAlphabet);
    void setInitialState(std::shared_ptr<State> newInitialState);
    void setStates(const std::vector<std::shared_ptr<State>>& newStates);
    void setFinalStates(const std::vector<std::shared_ptr<State>>& newFinalStates);
    void setTransitionsRelation(std::unique_ptr<TransitionsRelation> newTransitionsRelation);

    [[nodiscard]] std::shared_ptr<State> findStateBySubstates(const std::set<int>& substates) const;
    [[nodiscard]] bool isDeterministic() const;

    std::vector<std::string> getOutputOfTransition(const std::shared_ptr<State>& current,
                                                   const std::string& input) const;

    bool doesAcceptWord(const std::vector<std::string>& word) const;

private:
    std::unique_ptr<Alphabet> alphabet;
    std::unique_ptr<Alphabet> outAlphabet;
    std::shared_ptr<State> initialState;
    std::vector<std::shared_ptr<State>> states;
    std::vector<std::shared_ptr<State>> finalStates;
    std::unique_ptr<TransitionsRelation> transitionsRelation;
    OutputSymbolMap outSymbolsRelation;
};

#endif // AUTOMATA_H
