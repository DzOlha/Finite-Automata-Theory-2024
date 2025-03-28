//
// Created by dima on 17.09.24 (https://github.com/d1mmm)
//

#include <visualizer/AutomatonVisualizer.h>
#include <fstream>
#include <iostream>

/*
 * Add visualizer to Mealy automates. Use Graphviz https://graphviz.org/
 * Create an instruction in the form of .dot file.
 */


void AutomatonVisualizer::visualizeAutomaton(const Automaton& automate, const std::string& filename) {
    const auto dot_filename = filename + ".dot";
    std::ofstream dotFile(dot_filename);

    if (!dotFile.is_open()) {
        std::cerr << "File could not be opened for writing!" << std::endl;
        return;
    }

    dotFile << "digraph MealyAutomate {" << std::endl;
    dotFile << "rankdir=LR;" << std::endl;
    dotFile << "size=8.5;" << std::endl;
    dotFile << "    node [shape = point]; qinit;" << std::endl;
    dotFile << "node [shape = circle];" << std::endl;
    dotFile << "    qinit -> \"{" << automate.getInitialStates()[0]->getName() << "}\";" << std::endl;

    const auto finalStates = generateStatesVector(automate.getFinalStates());
    for(const auto& finalState : finalStates) {
        dotFile << finalState << " [shape = doublecircle];" << std::endl;
    }

    for (const auto& [key, nextStates] :
        automate.getTransitionsRelation()->getTransitions()) {
        const auto& from = key.first;
        std::string symbol = key.second;
        const auto fromStates = "\"{" + from->getName() + "}\"";;
        const auto toStates = generateStatesVector(nextStates);

        for(const auto& toState : toStates) {
            // std::cout << "    " << fromStates << " -> " << toState << " [label=\"" << symbol << " / [";
            dotFile << "    " << fromStates << " -> " << toState << " [label=\"" << symbol << " ";
            // const auto outSymbolIt = automate.getOutSymbolsRelation().find(key);
            // if (outSymbolIt != automate.getOutSymbolsRelation().end()){
            //     const std::set outSymbolsForState(outSymbolIt->second.cbegin(), outSymbolIt->second.cend());
            //     auto it = outSymbolsForState.begin();
            //     while (it != outSymbolsForState.end()) {
            //         dotFile << *it;
            //         ++it;
            //         if (it != outSymbolsForState.end()) {
            //             //std::cout << ", ";
            //             dotFile << ", ";
            //         }
            //     }
            // }
            dotFile << "\"];" << std::endl;
        }
    }

    dotFile << "}" << std::endl;
    dotFile.close();
    convertDotToPng(dot_filename, filename + ".png");
}

// Generate vector string from std::vector<std::shared_ptr<State>>
std::vector<std::string> AutomatonVisualizer::generateStatesVector(const std::vector<std::shared_ptr<State>>& states) {
    std::vector<std::string> vec_states;
    vec_states.reserve(states.size());
    for(const auto & state : states) {
        vec_states.push_back( "\"{" + state->getName() + "}\"");
    }
    return vec_states;
}

// Convert .dot file to the png
void AutomatonVisualizer::convertDotToPng(const std::string& dotFilePath, const std::string& outputPngPath) {
    if(!isDotInstalled()) {
        std::cout << "Graphviz is not installed on your PC. Please visit https://graphviz.org/download/\n";
        return;
    }
    const std::string command = "dot -Tpng " + dotFilePath + " -o " + outputPngPath;
    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "File converted successfully в PNG: " << outputPngPath << std::endl;
    } else {
        std::cerr << "File conversion error." << std::endl;
    }
}

// Check for installation Graphviz
bool AutomatonVisualizer::isDotInstalled() {
    #ifdef _WIN32
        return system("where dot >nul 2>nul") == 0;
    #else
        return system("which dot > /dev/null 2>&1") == 0;
    #endif
}
