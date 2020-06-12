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
#include <map>
#include <vector>
#include <stdexcept>
#include <unordered_set>
#include <string>
#include <sstream>

/**
 * @brief Generates Graphviz dot code for the cycle graph of the group.
 */
class CycleGraph {
    private:
        int order;
        int size;
        // Cycles are grouped by length
        std::map<int, std::list<std::list<int>>*> cycles;
        std::vector<uint32_t> cyclesByValues;

    public:
        CycleGraph(int order, uint8_t *cayley) : cycles(), cyclesByValues(order + 1) {
            this->order = order;
            this->size = order * order;

            for(int element = 2; element <= order; element++) {
                std::list<int> newCycle;
                int current = element;
                this->cyclesByValues[1] |= ((uint32_t)1) << (element - 1);

                /*
                    By Lagrange's theorem we should get back to
                    the identity element after some steps in a
                    power series. If not than the input is not
                    the Cayley table of a proper symmetry group.
                */
                int limit = this->order;

                while(current != 1) {
                    if (limit-- == 0) {
                        throw std::runtime_error("CycleGraph: The Cayley table is invalid.");
                    }

                    newCycle.push_back(current);
                    this->cyclesByValues[current] |= ((uint32_t)1) << (element - 1);

                    // current = current * element
                    current = *(cayley + (current - 1) * order + element - 1);
                }

                if (newCycle.size() > 0) {
                    /*
                        Store it
                    */
                    auto cycleGroupIter = this->cycles.find(newCycle.size());
                    std::list<std::list<int>> *cycleGroup;

                    if (cycleGroupIter == this->cycles.end()) {
                        cycleGroup = new std::list<std::list<int>>();
                        this->cycles.insert({newCycle.size(), cycleGroup});
                    } else {
                        cycleGroup = cycleGroupIter->second;
                    }

                    cycleGroup->push_back(newCycle);
                }
            }
        }

        ~CycleGraph() {
            for (const auto &iter : this->cycles) {
                delete iter.second;
            }
        }

        /**
         * @brief Generate text code for Graphviz.
         */
        std::string GetGraphVizCode() {
            std::stringstream code;
            uint32_t added = 0;
            auto cycleGroupIter = this->cycles.end();

            code << "strict graph Group {\n";
            code << "    node [shape=circle, fontsize=6, fixedsize=true, width=0.2]\n";
            code << "    1 [style=filled]\n\n";

            do {
                /*
                    Progress from the longest cycles to the shortest.
                */
                cycleGroupIter--;

                for(const auto &cycle : *(cycleGroupIter->second)) {
                    uint32_t cycleID =(uint32_t)1 << (*cycle.begin() - 1);

                    /*
                        Check if the cycle is already contained by another one.
                    */
                    uint32_t test = 0xFFFFFFFFU;
                    for(int element : cycle) {
                        test &= this->cyclesByValues[element];

                        if (!test) {
                            break;
                        }
                    }

                    if (test & added) {
                        continue;
                    }
                    
                    /*
                        Print cycle
                    */
                    added |= cycleID;
                    code << "    1 -- ";

                    for (int element : cycle) {
                        code << element << " -- ";
                    }

                    code << "1\n";
                }
            } while(cycleGroupIter != this->cycles.begin());

            code << "}\n";

            return code.str();
        }

        std::string GetCsAcademyCode() {
            std::stringstream code;
            uint32_t added = 0;
            auto cycleGroupIter = this->cycles.end();

            do {
                /*
                    Progress from the longest cycles to the shortest.
                */
                cycleGroupIter--;

                for(const auto &cycle : *(cycleGroupIter->second)) {
                    uint32_t cycleID =(uint32_t)1 << (*cycle.begin() - 1);

                    /*
                        Check if the cycle is already contained by another one.
                    */
                    uint32_t test = 0xFFFFFFFFU;
                    for(int element : cycle) {
                        test &= this->cyclesByValues[element];

                        if (!test) {
                            break;
                        }
                    }

                    if (test & added) {
                        continue;
                    }
                    
                    /*
                        Print cycle
                    */
                    added |= cycleID;

                    for(auto iter = cycle.begin(); iter != cycle.end(); iter++) {
                        if (iter == cycle.begin()) {
                            code << "1 " << *iter << '\n';
                        }

                        auto next = std::next(iter);

                        if (next == cycle.end()) {
                            code << *iter << " 1\n";
                            break;
                        }

                        code << *iter << ' ' << *next << '\n';
                    }

                    code << '\n';
                }

            } while(cycleGroupIter != this->cycles.begin());

            return code.str();
        }
};
