/*
    Copyright 2020 Tamas Bolner
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
      http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include <iostream>
#include "LatinHeuristics.hpp"
#include "AssocHeuristics.hpp"
#include "RandomHeuristics.hpp"
#include "CycleGraph.hpp"
#include "Classifier.hpp"

int main() {
    int order = 8;
    AssocHeuristics heuristics(order);

    while(true) {
        heuristics.Next();
        
        if (!heuristics.Found()) {
            std::cout << "\nNothing found.\n\n";
            return 0;
        }

        Classifier classifier(order, heuristics.GetCayley());
        std::cout << classifier.PrintGroup() << "\n\n";
        
        CycleGraph graph(order, heuristics.GetCayley());
        std::cout << graph.PrintCyclicSubgroups() << "\n";

        std::cout << "Properties: " << classifier.PrintAllProperties() << "\n\n";

        std::cout << "Press enter to continue...\n";
        std::cout << std::flush;

        std::cin.get();
    }
    
    return 0;
}
