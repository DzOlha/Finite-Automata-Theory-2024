//
// Created by dzuha on 03/11/2024.
//

#include "helper/VisualizeHelper.h"
#include "visualizer/AutomatonVisualizer.h"
#include <filesystem>

void VisualizeHelper::toImage(const Automaton& res, const std::string& filename, const int counter)
{
    // create an image of the automaton
    std::string folder = "./visualized";
    std::string outputDir =  folder + "/example_" + std::to_string(counter) + "/";
    std::filesystem::create_directories(outputDir);

    // std::cout << "\n|-------------------------------- EXAMPLE: " << "(x(x)*y)(xy)^" << " -----------------------------|\n";
    AutomatonVisualizer::visualizeAutomaton(res, outputDir + filename);
}
