//
// Created by dima on 17.09.24 (https://github.com/d1mmm)
//

#include <visualizer/AutomatonVisualizer.h>
#include <fstream>
#include <iostream>

/*
 * Add visualizer to nfa and dfa automates. Use Graphviz https://graphviz.org/
 * Create an instruction in the form of .dot file.
 * Get substates from automate transitions
 * Convert this to the string or vector string
 */


void AutomatonVisualizer::visualizeAutomaton(const Automaton& automate, const std::string& filename)
{
    const auto dot_filename = filename + ".dot";
    std::ofstream dotFile(dot_filename);

    dotFile << "digraph Automate {" << std::endl;
    dotFile << "rankdir=LR;" << std::endl;
    dotFile << "node [shape = circle];" << std::endl;
    dotFile << "start [shape=point];" << std::endl;
    dotFile << "start -> " << "\"{" << automate.getInitialState()->getName() << "}\"" << ";" << std::endl;

    for (const auto& [key, nextStates] :
         automate.getTransitionsRelation()->getTransitions())
    {
        const std::set<int> from = key.first->getSubstates();
        char symbol = key.second;
        const auto fromStates = generateStatesString(from);
        const auto toStates = generateStatesVector(nextStates);

        for (const auto& toState : toStates)
        {
            dotFile << fromStates << " -> " << toState << " [label=\"" << symbol << "\"];" << std::endl;
        }
    }

    const auto finalStates = generateStatesVector(automate.getFinalStates());
    for (const auto& finalState : finalStates)
    {
        dotFile << finalState << " [shape = doublecircle];" << std::endl;
    }

    dotFile << "}" << std::endl;
    dotFile.close();
    convertDotToPng(dot_filename, filename + ".png");
}

// Generate string from set string
std::string AutomatonVisualizer::generateStatesString(const std::set<int>& states)
{
    std::string name = "\"{";
    auto it = states.begin();
    while (it != states.end())
    {
        name += std::to_string(*it);
        ++it;
        if (it != states.end())
        {
            name += ",";
        }
    }
    name += "}\"";
    return name;
}

// Generate vector string from std::vector<std::shared_ptr<State>>
std::vector<std::string> AutomatonVisualizer::generateStatesVector(const std::vector<std::shared_ptr<State>>& states)
{
    std::vector<std::string> vec_states;
    vec_states.reserve(states.size());
    for (const auto& state : states)
    {
        vec_states.push_back("\"{" + state->getName() + "}\"");
    }
    return vec_states;
}

// Convert .dot file to the png
void AutomatonVisualizer::convertDotToPng(const std::string& dotFilePath, const std::string& outputPngPath)
{
    if (!isDotInstalled())
    {
        std::cout << "Graphviz is not installed on your PC. Please visit https://graphviz.org/download/\n";
        return;
    }
    const std::string command = "dot -Tpng " + dotFilePath + " -o " + outputPngPath;
    int result = system(command.c_str());
    if (result == 0)
    {
        std::cout << "File converted successfully в PNG: " << outputPngPath << std::endl;
    }
    else
    {
        std::cerr << "File conversion error." << std::endl;
    }
}

// Check for installation Graphviz
bool AutomatonVisualizer::isDotInstalled()
{
#ifdef _WIN32
    return system("where dot >nul 2>nul") == 0;
#else
        return system("which dot > /dev/null 2>&1") == 0;
#endif
}
