#ifndef STATE_H
#define STATE_H

#include <memory>
#include <vector>
#include <string>
#include <set>

/**
 * State is tuple (std::vector) of ElementaryState.
 * Elementary state is set of substates (integer)
 *
 * If you want to display simple state, it will vector with only one ElementaryState
 */
class State {
private:
    std::vector<std::set<int>> elementaryStates;
    bool final;
    std::string secondaryName;


private:
    [[nodiscard]] static std::string vectorToString(const std::vector<int>& elements, char delimiter);
    [[nodiscard]] static std::string setToString(const std::set<int>& elements, char delimiter);

    
public:
    explicit State(std::vector<std::set<int>> elementaryStates, bool final = false);
    explicit State(const std::vector<std::shared_ptr<State>>& elementaryStates, bool final = false);

    void setElementaryStates(const std::vector<std::set<int>>& newElementaryStates);
    void setFinal(bool final = true);
    void setSecondaryName(const std::string &secondary_name);

    [[nodiscard]] std::vector<std::set<int>> getElementaryStates() const;
    [[nodiscard]] bool isFinal() const;
    [[nodiscard]] std::string getName() const; ///////
    [[nodiscard]] bool isElementary() const;
    [[nodiscard]] std::string getSecondaryName() const;

    bool operator==(const State& other) const;
};

namespace std {
    template <>
    struct hash<State> {
        size_t operator()(const State& state) const;
    };
}

#endif // STATE_H
