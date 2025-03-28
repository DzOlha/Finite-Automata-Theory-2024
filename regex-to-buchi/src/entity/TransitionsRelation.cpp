#include <entity/TransitionsRelation.h>
#include <utility>

template <class T1, class T2>
size_t PairHash::operator()(const std::pair<std::shared_ptr<T1>, T2>& p) const {
    auto hash1 = std::hash<std::shared_ptr<T1>>{}(p.first);
    auto hash2 = std::hash<T2>{}(p.second);
    return hash1 ^ (hash2 << 1); // Combine the two hash values
}

TransitionsRelation::TransitionsRelation(TransitionMap transitions)
    : transitions(std::move(transitions)) {}

std::vector<std::shared_ptr<State>> TransitionsRelation::move(
    const std::shared_ptr<State>& current, const std::string& input
    ) const {
    TransitionKey key = std::make_pair(current, input);
    auto it = transitions.find(key);
    if (it != transitions.end()) {
        return it->second;
    }
    return {}; // Return an empty vector if no transition exists
}

[[nodiscard]] const TransitionsRelation::TransitionMap& TransitionsRelation::getTransitions() const {
    return transitions;
}

/*
    Inserts new mapping (startState, input) -> endState, if it didn't exist.
*/
void TransitionsRelation::addTransition(
    const std::shared_ptr<State>& startState,
    const std::string& input,
    const std::shared_ptr<State>& endState
    ) {
    TransitionKey key = std::make_pair(startState, input);

    auto search = transitions.find(key); // pair (key,vector<shared_ptr<State>>)
    if (search != transitions.end()) { // if key mapping exists
        for (const auto& s : search->second) { // check if transition with same endState is already here
            if (s->getElementaryStates() == endState->getElementaryStates() && s->getSecondaryName() == endState->getSecondaryName()) {
                return; // no actions required, because desired transition is already here
            }
        }
        transitions[key].push_back(endState);
    }
    else { // there are no transitions from startState on input
        transitions.insert({ key,  {endState} });
    }
}

void TransitionsRelation::setTransitions(const TransitionMap& pairs)
{
    transitions = pairs;
}
