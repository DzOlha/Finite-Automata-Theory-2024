//
// Created by dzuha on 03/11/2024.
//

#ifndef VISUALIZEHELPER_H
#define VISUALIZEHELPER_H
#include <string>
#include "entity/Automaton.h"

class VisualizeHelper
{
    public:
      static void toImage(const Automaton& res, const std::string& filename = "example", int counter = 1);
};

#endif //VISUALIZEHELPER_H
