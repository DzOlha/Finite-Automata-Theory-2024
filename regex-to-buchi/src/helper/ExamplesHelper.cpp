//
// Created by dzuha on 03/11/2024.
//
#include <iostream>
#include <helper/ExamplesHelper.h>
#include <helper/ConcatenationHelper.h>
#include <helper/VisualizeHelper.h>

void ExamplesHelper::processExamples()
{
    std::vector<std::vector<std::string>> examples = {
        example1(),
        example2(),
        example3(),
        example4()
    };

    for (size_t i = 0; i < examples.size(); ++i) {
        std::cout << "\n|-------------------------------- Concatenation Example " << i + 1 << ": "<< "lang1 = " << examples[i][0] << ", lang2 = " << examples[i][1] << " -----------------------------|\n";

        auto res = ConcatenationHelper::concatenate(examples[i][0], examples[i][1]);
        res->print();

        std::string filename = "concat_example_" + std::to_string(i+1);
        VisualizeHelper::toImage(*res, filename, i);
    }
    std::cout << "\n|------------------------------------------------------------------------------|\n";
}

std::vector<std::string> ExamplesHelper::example1()
{
    std::string regex1 = "(x(x)*y)";
    std::string regex2 = "(xy)^";

    return {regex1, regex2};
}

std::vector<std::string> ExamplesHelper::example2()
{
    std::string regex1 = "x|y";
    std::string regex2 = "y(x)^";

    return {regex1, regex2};
}

std::vector<std::string> ExamplesHelper::example3()
{
    std::string regex1 = "a(b|c)";
    std::string regex2 = "(c|d)a^";

    return {regex1, regex2};
}

std::vector<std::string> ExamplesHelper::example4()
{
    std::string regex1 = "xy";
    std::string regex2 = "z^";

    return {regex1, regex2};
}