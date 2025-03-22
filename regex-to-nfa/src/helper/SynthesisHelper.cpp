#include <iostream>
#include <algo/RegexToNFA.h>
#include <helper/SynthesisHelper.h>
#include <helper/VisualizeHelper.h>

std::unique_ptr<Automaton> SynthesisHelper::getNFA(const std::string& regex, bool print)
{
    RegexToNFA converter(regex);
    std::unique_ptr<Automaton> nfa = converter.convert();

    if (print) nfa->print();

    RegexToNFA::removeEpsilonTransitions(nfa);

    if (print)
    {
        std::cout << "\n\n\n";
        nfa->print();
    }

    return nfa;
}

void SynthesisHelper::examples()
{
    std::cout << "\n|------------------------------ START: Synthesis Example -----------------------------|\n";
    std::string regex1 = "(x|y)*x|x*y*";
    std::string regex2 = "((0)*1)*";
    std::string regex3 = "xy*|y|yx|xx";

    std::vector<std::string> regex = {regex1, regex2, regex3};

    for (int i = 0; i < regex.size(); i++)
    {
        auto automate1 = SynthesisHelper::getNFA(regex[i]);
        std::cout << "Regex: " << regex[i] << std::endl;
        automate1->print();

        VisualizeHelper::toImage(*automate1, "synthesized", i + 1);
        std::cout << "\n\n";
    }

    std::cout << "\n|------------------------------ END: Synthesis Example -----------------------------|\n";
}
