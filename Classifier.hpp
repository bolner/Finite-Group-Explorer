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

#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>
#include "Combinator.hpp"

class Classifier {
    private:
        int order;
        uint8_t *cayley;
        std::string message;
    
    public:
        Classifier(int order, uint8_t *cayley) {
            this->order = order;
            this->cayley = cayley;
        }

        bool IsAssociative() {
            uint8_t left, right, x;

            for(uint8_t i = 0; i < this->order; i++) {
                for(uint8_t j = 0; j < this->order; j++) {
                    for(uint8_t k = 0; k < this->order; k++) {
                        // (i*j)*k
                        x = *(this->cayley + i * this->order + j) - 1;
                        left = *(this->cayley + x * this->order + k);

                        // i*(j*k)
                        x = *(this->cayley + j * this->order + k) - 1;
                        right = *(this->cayley + i * this->order + x);

                        if (left != right) {
                            this->message = "Not associative. (" + std::to_string(i + 1) + " * " + std::to_string(j + 1)
                                + ") * " + std::to_string(k + 1) + " != " + std::to_string(i + 1) + " * ("
                                + std::to_string(j + 1) + " * " + std::to_string(k + 1) + ")";
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        std::string GetMessage() {
            return this->message;
        }

        bool IsAbelian() {
            for(uint8_t i = 0; i < this->order; i++) {
                for(uint8_t j = i + 1; j < this->order; j++) {
                    if (*(this->cayley + i*order + j) != *(this->cayley + j*order + i) ) {
                        this->message = "Non-abelian. " + std::to_string(i + 1) + " * " + std::to_string(j + 1)
                            + " != " + std::to_string(j + 1) + " * " + std::to_string(i + 1);
                        return false;
                    }
                }
            }

            return true;
        }

        /**
         * @brief Returns the proper non-trivial subgroups of the group.
         */
        std::vector<std::vector<uint8_t>> GetSubGroups() {
            std::vector<std::vector<uint8_t>> subgroups;
            int checkTo = this->order >> 1;
            std::vector<uint8_t> v(this->order, 0);
            int i, j, k;
            
            for(int subgroupOrder = 2; subgroupOrder <= checkTo; subgroupOrder++) {
                if (this->order % subgroupOrder != 0) {
                    /*
                        Lagrange's theorem
                    */
                    continue;
                }

                /*
                    Select all combinations of "subgroupOrder" number
                    of elements, and see if they are closed under
                    the group operation. We don't need to check
                    inversion and associativity, because those
                    properties can't change if we take a subset.
                */
                Combinator combi(this->order, subgroupOrder);
                bool notFound;

                while(combi.Next(v)) {
                    for(i = 0; i < subgroupOrder; i++) {
                        for(j = 0; j < subgroupOrder; j++) {
                            uint8_t value = *(this->cayley + v[i] * this->order + v[j]) - 1;
                            notFound = true;

                            for(k = 0; k < subgroupOrder; k++) {
                                if (v[k] == value) {
                                    notFound = false;
                                    break;
                                }
                            }

                            if (notFound) {
                                goto nextCombination;
                            }
                        }
                    }

                    /*
                        It is a proper subgroup.
                        Store it.
                    */
                    for (i = 0; i < subgroupOrder; i++) {
                        v[i]++; // convert the indices to group values
                    }
                    subgroups.push_back(v);

                    nextCombination: ;
                }
            }

            return subgroups;
        }
/*
        std::vector<std::vector<uint8_t>> GetNormalSubGroups() {
            
        }
*/
        bool IsSimple() {
            

            return false;
        }

        std::string PrintSubgroups(std::vector<std::vector<uint8_t>> &subgroups) {
            std::stringstream o;

            if (subgroups.size() < 1) {
                return std::string();
            }

            for(const auto &group : subgroups) {
                o << "\n| * |";

                for(const uint8_t &e1 : group) {
                    o << (int)e1 << "|";
                }

                o << "\n|";

                for(unsigned int i = 0; i <= group.size(); i++) {
                    o << " - |";
                }

                o << "\n";

                for(const uint8_t &e1 : group) {
                    o << "|<b>" << (int)e1 << "</b>|";

                    for(const uint8_t &e2 : group) {
                        o << (int)this->cayley[(e1 - 1) * this->order + e2 - 1] << "|";
                    }

                    o << '\n';
                }
            }

            return o.str();
        }

        std::string PrintGroup() {
            std::stringstream o;

            o << "\n| * |";

            for(int i = 1; i <= this->order; i++) {
                o << i << "|";
            }

            o << "\n|";

            for(int i = 0; i <= this->order; i++) {
                o << " - |";
            }

            o << "\n";

            for(int i = 0; i < this->order; i++) {
                o << "|<b>" << (i + 1) << "</b>|";

                for(int j = 0; j < this->order; j++) {
                    o << (int)this->cayley[i * this->order + j] << "|";
                }

                o << '\n';
            }

            return o.str();
        }
};
