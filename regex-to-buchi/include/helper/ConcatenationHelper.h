//
// Created by dzuha on 02/11/2024.
//

#ifndef CONCATENATIONHELPER_H
#define CONCATENATIONHELPER_H
#include <memory>

#include "entity/Automaton.h"

class ConcatenationHelper
{
    public:
        static std::unique_ptr<Automaton> concatenate(
            const std::string& omegaRegex1, const std::string& omegaRegex2
        );
};

#endif //CONCATENATIONHELPER_H
