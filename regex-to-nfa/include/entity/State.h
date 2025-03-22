#ifndef STATE_H
#define STATE_H

#include <vector>
#include <string>
#include <set>

class State {
private:
    std::set<int> substates;
    bool final;

    [[nodiscard]] static std::string implode(const std::vector<int>& elements, char delimiter);

public:
    explicit State(std::set<int> substates, bool final = false);

    [[nodiscard]] bool isFinal() const;
    void setFinal(bool final = true);
    [[nodiscard]] std::string getName() const;

    bool isAtomic() const;
    const std::set<int> getSubstates() const;
    void setSubstates(const std::set<int>& newSubstates);

    bool operator==(const State& other) const;
};

namespace std {
    template <>
    struct hash<State> {
        size_t operator()(const State& state) const;
    };
}

#endif // STATE_H
