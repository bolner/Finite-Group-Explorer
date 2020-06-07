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
#include "CycleGraph.hpp"
#include "Classifier.hpp"

int main() {
    AssocHeuristics heuristics(16);
    
    // Problems: 53

    for(int i = 0; i < 210; i++) {
        heuristics.Next();
    }

    std::cout << heuristics.GetAsMarkdown() << "\n\n";
    
    CycleGraph graph(16, heuristics.GetCayley());
    std::cout << graph.GetGraphVizCode() << "\n";

    /*
        Check associativity for testing only, because
        the AssocHeuristics should already guarantee
        that.
    */
    Classifier classifier(16, heuristics.GetCayley());
    if (!classifier.IsAssociative()) {
        std::cout << "\n" << classifier.GetMessage() << "\n\n";
    }

    if (!classifier.IsAbelian()) {
        std::cout << "\n" << classifier.GetMessage() << "\n\n";
    } else {
        std::cout << "\nThe group is abelian.\n\n";
    }
    
    return 0;
}
