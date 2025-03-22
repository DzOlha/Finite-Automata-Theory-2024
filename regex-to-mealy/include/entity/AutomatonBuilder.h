#ifndef AUTOMATABUILDER_H
#define AUTOMATABUILDER_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <entity/State.h>
#include <entity/Automaton.h>
#include <entity/TransitionsRelation.h>

class AutomatonBuilder
{
private:
    std::unordered_map<int, std::shared_ptr<State>> stateMap;
    std::vector<std::shared_ptr<State>> states;
    std::vector<std::shared_ptr<State>> finalStates;
    std::vector<std::string> alphabet;
    std::vector<std::string> outAlphabet;
    std::unordered_map<
        std::pair<std::shared_ptr<State>, std::string>, std::vector<std::shared_ptr<State>>, PairHash
    > transitions;
    std::shared_ptr<State> initialState;

    std::unordered_map<
        std::pair<std::shared_ptr<State>, std::string>, std::vector<std::string>, PairHash
    > outs;

    std::shared_ptr<State> getState(int stateId);

public:
    AutomatonBuilder& setInitialState(int stateId);
    AutomatonBuilder& setFinalStates(const std::vector<int>& finalStateIds);
    AutomatonBuilder& setAlphabet(const std::vector<std::string>& alpha);
    AutomatonBuilder& setOutAlphabet(const std::vector<std::string>& alpha);
    AutomatonBuilder& setStates(const std::vector<int>& stateIds);

    AutomatonBuilder& setTransitions(
        const std::vector<
            std::tuple<int, std::vector<std::string>, int, std::vector<std::string>>
        >& trans);

    Automaton build();
};

#endif // AUTOMATABUILDER_H
