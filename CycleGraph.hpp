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
#pragma once

#include <stdint.h>
#include <list>
#include <unordered_set>
#include <string>
#include <sstream>

class CycleGraph {
    private:
        int order;
        int size;
        std::list<std::list<int>> cycles;

    public:
        CycleGraph(int order, uint8_t *cayley) : cycles() {
            this->order = order;
            this->size = order * order;

            for(int element = 2; element <= order; element++) {
                std::list<int> newCycle;
                newCycle.push_back(1);
                int current = element;

                /*
                    By Lagrange's theorem we should get back to
                    the identity element after some steps in a
                    power series. If not than the input is not
                    the Cayley table of a proper symmetry group.
                */
                while(current != 1) {
                    newCycle.push_back(current);

                    // current = current * element
                    current = *(cayley + (current - 1) * order + element - 1);
                }

                if (newCycle.size() > 2) {
                    newCycle.push_back(1);
                }

                if (newCycle.size() > 1) {
                    this->cycles.push_back(newCycle);
                }
            }
        }

        /**
         * @brief Generate text code for Graphviz.
         */
        std::string GetGraphVizCode() {
            std::stringstream code;

            code << "strict graph Group {\n";
            code << "    node [shape=circle, fontsize=6, fixedsize=true, width=0.2]\n";
            code << "    1 [style=filled]\n\n";

            for(const auto &cycle : this->cycles) {
                code << "    ";

                for (auto iter = cycle.begin(); iter != cycle.end(); ++iter) {
                    code << *iter;

                    if (std::next(iter) != cycle.end()) {
                        code << " -- ";
                    }
                }

                code << '\n';
            }

            code << "}\n";

            return code.str();
        }
};
