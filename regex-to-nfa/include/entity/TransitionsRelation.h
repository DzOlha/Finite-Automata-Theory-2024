#ifndef TRANSITION_H
#define TRANSITION_H

#include <entity/State.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>

struct PairHash {
    template <class T1, class T2>
    size_t operator()(const std::pair<std::shared_ptr<T1>, T2>& p) const;
};

class TransitionsRelation {
public:
    using TransitionKey = std::pair<std::shared_ptr<State>, char>;
    using TransitionMap = std::unordered_map<TransitionKey, std::vector<std::shared_ptr<State>>, PairHash>;

    explicit TransitionsRelation(TransitionMap transitions);
    std::vector<std::shared_ptr<State>> move(const std::shared_ptr<State>& current, char input) const;

    [[nodiscard]] const TransitionMap& getTransitions() const;

    void addTransition(const std::shared_ptr<State>& startState, char input, const std::shared_ptr<State>& endState);
    void setTransitions(const TransitionMap& pairs);

private:
    TransitionMap transitions;
};

#endif // TRANSITION_H
