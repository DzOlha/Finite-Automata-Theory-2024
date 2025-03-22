//
// Created by dzuha on 18/09/2024.
//

#ifndef SYNTHESISHELPER_H
#define SYNTHESISHELPER_H
#include <memory>
#include <string>
#include <entity/Automaton.h>

class SynthesisHelper
{
public:
    static std::unique_ptr<Automaton> getMealy(const std::string& regex, bool print = false, int i = 0);
    static void examples();
};

#endif //SYNTHESISHELPER_H
