#ifndef AUTOMATABUILDER_H
#define AUTOMATABUILDER_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <entity/State.h>
#include <entity/Automaton.h>
#include <entity/TransitionsRelation.h>

struct VectorSetHash {
    std::size_t operator()(const std::vector<std::set<int>>& vecSet) const {
        State stateFromVector{vecSet, false};

        std::hash<State> stateHashFunction; // using State hash function, defined in State.cpp
        return stateHashFunction(stateFromVector);
    }
};


class AutomatonBuilder
{
private:
    std::unordered_map<std::vector<std::set<int>>, std::shared_ptr<State>, VectorSetHash> stateMap;
    std::vector<std::shared_ptr<State>> states;
    std::vector<std::shared_ptr<State>> finalStates;
    std::vector<std::string> alphabet;
    std::vector<std::string> outAlphabet;
    std::unordered_map<
        std::pair<std::shared_ptr<State>, std::string>, std::vector<std::shared_ptr<State>>, PairHash
    > transitions;
    std::vector<std::shared_ptr<State>> initialStates;

    std::unordered_map<
        std::pair<std::shared_ptr<State>, std::string>, std::vector<std::string>, PairHash
    > outs;


private:
    std::shared_ptr<State> getState(const std::vector<std::set<int>>& stateId);


public:
    AutomatonBuilder& setStates(const std::vector<std::vector<std::set<int>>>& stateIds);
    AutomatonBuilder& setInitialStates(const std::vector<std::vector<std::set<int>>>& initialStateIds);
    AutomatonBuilder& setFinalStates(const std::vector<std::vector<std::set<int>>>& finalStateIds);
    AutomatonBuilder& setAlphabet(const std::vector<std::string>& alpha);
    AutomatonBuilder& setOutAlphabet(const std::vector<std::string>& alpha);

    AutomatonBuilder& setTransitions(
        const std::vector<
                std::tuple<std::vector<std::set<int>>,
                            std::vector<std::string>,
                            std::vector<std::set<int>>,
                            std::vector<std::string>
                            >
            >& trans);

    Automaton build();
};

#endif // AUTOMATABUILDER_H
