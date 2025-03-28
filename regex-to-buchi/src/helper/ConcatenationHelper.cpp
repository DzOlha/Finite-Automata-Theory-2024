//
// Created by dzuha on 02/11/2024.
//

#include "helper/ConcatenationHelper.h"

#include "algo/RegexToNFA.h"

std::unique_ptr<Automaton> ConcatenationHelper::concatenate(
    const std::string& omegaRegex1, const std::string& omegaRegex2
){
    // make automata by regexes
    auto converter1 = new RegexToNFA(omegaRegex1);
    auto converter2 = new RegexToNFA(omegaRegex2);

    auto automaton1 = converter1->convertToNFA();
    // automaton1->print();

    auto automaton2 = converter2->convertToNFA();
    // automaton2->print();

    // concatenate them
    auto result = converter1->makeConcatenatedAutomaton(
        automaton1, automaton2
    );

    return result;
}
