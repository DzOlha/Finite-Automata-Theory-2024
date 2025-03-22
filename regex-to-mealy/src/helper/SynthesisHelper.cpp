#include <helper/SynthesisHelper.h>
#include <iostream>
#include <algo/RegexToMealy.h>
#include <helper/VisualizeHelper.h>

std::unique_ptr<Automaton> SynthesisHelper::getMealy(const std::string& regex, bool print, int i)
{
    RegexToMealy converter(regex);
    std::unique_ptr<Automaton> nfa = converter.convert();

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
    std::string regex1 = "([x/a][x/b])*[y/c]|[x/d]*|[x/a][x/c]";
    std::string regex2 = "([x/a])*|[y/c]";

    std::vector<std::string> regex = {regex1, regex2};

    for (int i = 0; i < regex.size(); i++)
    {
        auto automate1 = SynthesisHelper::getMealy(regex[i], false, i);
        std::cout << "Regex: " << regex[i] << std::endl;
        automate1->print();

        VisualizeHelper::toImage(*automate1, "synthesized", i + 1);
        std::cout << "\n\n";
    }

    std::cout << "\n|------------------------------ END: Synthesis Example -----------------------------|\n";
}
