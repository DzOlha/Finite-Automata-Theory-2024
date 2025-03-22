//
// Created by dima on 17.09.24.
//

#ifndef AUTOMATEVISUALIZER_H
#define AUTOMATEVISUALIZER_H

#include <entity/Automaton.h>

class AutomatonVisualizer
{
public:
    static void visualizeAutomaton(const Automaton& automate, const std::string& filename);

private:
    [[nodiscard]] static std::vector<std::string> generateStatesVector(
        const std::vector<std::shared_ptr<State>>& states);
    static void convertDotToPng(const std::string& dotFilePath, const std::string& outputPngPath);
    static bool isDotInstalled();
};

#endif //AUTOMATEVISUALIZER_H
