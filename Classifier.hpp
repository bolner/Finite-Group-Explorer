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
#include <set>
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

        std::string GetMessage() const {
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
        std::vector<std::vector<uint8_t>> GetSubGroups() const {
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

        bool IsSubGroupNormal(const std::vector<uint8_t> &subgroup) const {
            for(int g = 0; g < this->order; g++) {
                /*
                    Find inverse of g
                */
                int inv;

                for(inv = 0; inv < this->order; inv++) {
                    if (*(this->cayley + inv * this->order + g) == 1) {
                        goto foundInverse;
                    }
                }

                throw std::runtime_error("Element " + std::to_string(g + 1) + " has no inverse.");

                foundInverse:

                /*
                    Check if all g*n*inv element of the subgroup.
                    (The conjugate for all elements in the subgroup stays there.
                    Left and right cosets equal.)
                */
                for(int n : subgroup) {
                    int gn = *(this->cayley + g * this->order + n - 1) - 1;
                    int gni = *(this->cayley + gn * this->order + inv);

                    for(int n2 : subgroup) {
                        if (n2 == gni) {
                            goto checkNext;
                        }
                    }

                    return false;

                    checkNext: ;
                }
            }

            return true;
        }

        bool IsSimple() const {
            auto subgoups = this->GetSubGroups();

            for(const auto &subgroup : subgoups) {
                if (this->IsSubGroupNormal(subgroup)) {
                    return false;
                }
            }

            return true;
        }

        std::vector<std::vector<uint8_t>> GetNormalSubGroups() const {
            std::vector<std::vector<uint8_t>> normalSubgroups;
            auto subgoups = this->GetSubGroups();

            for(const auto &subgroup : subgoups) {
                if (this->IsSubGroupNormal(subgroup)) {
                    normalSubgroups.push_back(subgroup);
                }
            }

            return normalSubgroups;
        }

        std::string PrintSubgroups(std::vector<std::vector<uint8_t>> &subgroups) const {
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
                    o << " --- |";
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

        std::string PrintGroup() const {
            std::stringstream o;

            o << "\n| * |";

            for(int i = 1; i <= this->order; i++) {
                o << i << "|";
            }

            o << "\n|";

            for(int i = 0; i <= this->order; i++) {
                o << " --- |";
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

        bool IsDedekind() {
            auto subgoups = this->GetSubGroups();

            for(const auto &subgroup : subgoups) {
                if (!this->IsSubGroupNormal(subgroup)) {
                    return false;
                }
            }

            return true;
        }

        bool IsHamiltonian() {
            if (this->IsAbelian()) {
                return false;
            }

            return this->IsDedekind();
        }

        /**
         * @brief A group is cyclic if it contains
         * at least one element that can generate
         * the group through a power sequence.
         */
        bool IsCyclic() {
            uint32_t goal = (((uint32_t)1) << this->order) - 1;

            // Skip the identity element 0
            for(int g = 1; g < this->order; g++) {
                uint32_t bitMask = 1 | (((uint32_t)1) << g);
                int e = g;
                
                for(int n = 0; n < this->order; n++) {
                    // e = e * g
                    e = *(this->cayley + e * this->order + g) - 1;
                    if (e == 0) {
                        break;
                    }

                    bitMask |= ((uint32_t)1) << e;
                }

                if (bitMask == goal) {
                    return true;
                }
            }

            return false;
        }

        std::string PrintAllProperties() {
            std::stringstream o;

            if (!this->IsAssociative()) {
                o << "Not associative. ";
            }

            if (this->IsAbelian()) {
                o << "Abelian.";
            } else {
                o << "Non-abelian.";
            }

            if (this->IsCyclic()) {
                o << " Cyclic.";
            }

            if (this->IsSimple()) {
                o << " Simple.";
            }

            if (this->IsHamiltonian()) {
                o << " Hamiltonian.";
            }
            else if (this->IsDedekind()) {
                o << " Dedekind.";
            }

            return o.str();
        }
};
