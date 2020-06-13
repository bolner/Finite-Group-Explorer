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
#include <bitset>
#include <iomanip>
#include <sstream>
#include <string.h>

/**
 * @brief Find groups (Use the associative property in the heuristic search.)
 */
class AssocHeuristics {
    private:
        int order;
        int size;
        uint8_t *cayley;         // Cayley table
        uint32_t *track;         // Bitmaps of already used values.
        uint32_t *rowValues;     // Bitmaps of currently used values in rows.
        uint32_t *columnValues;  // Bitmaps of currently used values in columns.
        int x;
        int y;
        int pos;
        bool found;
        
        inline bool StepForward() {
            if (this->x >= this->order - 1) {
                if (this->y >= this->order - 1) {
                    return false;
                }
                
                this->x = 1;
                this->y++;
                this->pos += 2;
            } else {
                this->x++;
                this->pos++;
            }

            return true;
        }

        inline bool StepBackward() {
            if (this->x <= 1) {
                if (this->y <= 1) {
                    return false;
                }
                
                this->x = this->order - 1;
                this->y--;
                this->pos -= 2;
            } else {
                this->x--;
                this->pos--;
            }

            return true;
        }

        inline void Set(uint8_t value) {
            uint8_t normalValue = value - 1;
            uint32_t bit = ((uint32_t)1) << normalValue;

            this->cayley[this->pos] = value;
            this->track[this->pos] |= bit;
            this->rowValues[this->y] |= bit;
            this->columnValues[this->x] |= bit;

            /*std::cout << "Columns[4]: " << std::bitset<32>(this->columnValues[4]) << '\n';
            std::cout << "Rows[1]: " << std::bitset<32>(this->rowValues[1]) << '\n';
            std::cout << "Track[9]: " << std::bitset<32>(this->track[9]) << '\n';
            */
            
            // std::cout << this->GetAsText(true) << '\n';
        }

        inline void Unset(bool isBackTracking) {
            if (isBackTracking) {
                this->track[this->pos] = 0;
            }

            uint8_t value = this->cayley[this->pos];
            if (value == 0) {
                // Nothing to unset
                // std::cout << this->GetAsText(true) << '\n';
                return;
            }

            uint8_t normalValue = value - 1;
            uint32_t bit = ~(((uint32_t)1) << normalValue);

            this->cayley[this->pos] = 0;
            this->rowValues[this->y] &= bit;
            this->columnValues[this->x] &= bit;

            // std::cout << this->GetAsText(true) << '\n';
        }

        inline uint8_t Mult(uint8_t a, uint8_t b) {
            return this->cayley[a * this->order + b];
        }

        inline int FindPossibleValue() {
            uint8_t oldValue = this->cayley[this->pos];

            start_find:
            
            uint32_t bitMap = ~(this->track[this->pos] | this->rowValues[this->y] | this->columnValues[this->x]);
            // https://gcc.gnu.org/onlinedocs/gcc-4.8.0/gcc/Other-Builtins.html
            int value = __builtin_ffs(bitMap);

            if (value > this->order) {
                this->cayley[this->pos] = oldValue;
                return value;
            }

            this->cayley[this->pos] = value;
            int normalValue = value - 1;
            uint8_t left, x_i, right, i_y;

            for(uint8_t i = 0; i < this->order; i++) {
                /*
                    Right associative checks (y*x)*i = y*(x*i)
                */
                left = this->Mult(normalValue, i);
                if (left == 0) {
                    goto left_checks;   // No information
                }
                
                x_i = this->Mult(this->x, i);
                if (x_i == 0) {
                    goto left_checks;   // No information
                }

                right = this->Mult(this->y, x_i - 1);
                if (right == 0) {
                    goto left_checks;   // No information
                }

                if (left != right) {
                    this->track[this->pos] |= ((uint32_t)1) << normalValue;
                    goto start_find;
                }

                /*
                    Left associative checks i*(y*x) = (i*y)*x
                */
                left_checks:

                left = this->Mult(i, normalValue);
                if (left == 0) {
                    continue;   // No information
                }
                
                i_y = this->Mult(i, this->y);
                if (i_y == 0) {
                    continue;   // No information
                }

                right = this->Mult(i_y - 1, this->x);
                if (right == 0) {
                    continue;   // No information
                }

                if (left != right) {
                    this->track[this->pos] |= ((uint32_t)1) << normalValue;
                    goto start_find;
                }
            }

            this->cayley[this->pos] = oldValue;

            return value;
        }

        inline uint32_t BackTracking() {
            int next;

            while(this->StepBackward()) {
                next = this->FindPossibleValue();
                if (next <= this->order) {
                    this->Unset(false);
                    return next;
                }

                this->Unset(true);
            }

            return this->order + 1;
        }

    public:
        AssocHeuristics(uint8_t order) {
            if (order < 2 || order > 31) {
                throw std::runtime_error("Invalid order value. Allowed: 2 -> 31");
            }

            this->order = order;
            this->size = order * order;
            this->cayley = new uint8_t[this->size];
            this->track = new uint32_t[this->size];
            this->rowValues = new uint32_t[order];
            this->columnValues = new uint32_t[order];
            this->x = 1;
            this->y = 1;
            this->pos = this->order + 1;
            this->found = false;

            memset(this->cayley, 0, this->size * sizeof(uint8_t));
            memset(this->track, 0, this->size * sizeof(uint32_t));
            memset(this->rowValues, 0, order * sizeof(uint32_t));
            memset(this->columnValues, 0, order * sizeof(uint32_t));

            /*
                Fixed values
            */
            for(int i = 0; i < this->order; i++) {
                /*
                    Horizontal
                */
                *(this->cayley + i) = i + 1;
                this->columnValues[i] |= 1 << i;

                /*
                    Vertical
                */
                *(this->cayley + i * this->order) = i + 1;
                this->rowValues[i] |= 1 << i;
            }
        }

        ~AssocHeuristics() {
            delete[] this->cayley;
            delete[] this->track;
            delete[] this->rowValues;
            delete[] this->columnValues;
        }

        bool Next() {
            int next;
            this->found = false;

            do {
                /*
                    Search for a possible value
                */
                next = this->FindPossibleValue();
                
                if (next > this->order) {
                    /*
                        No value left to be tried => backtracking
                    */
                    this->Unset(true);
                    next = this->BackTracking();
                    if (next > this->order) {
                        return false;
                    }
                }

                this->Unset(false);
                this->Set(next);

                // std::cin.get();

            } while(this->StepForward());

            this->found = true;

            if (next > this->order) {
                // Last one
                return false;
            }

            // More to be expected
            return true;
        }

        std::string GetAsText(bool showTrack = false) {
            std::stringstream result;

            for(int i = 0; i < this->order; i++) {
                for(int j = 0; j < this->order; j++) {
                    result << std::setfill('0') << std::setw(2)
                        << (int)(*(this->cayley + j + i * this->order)) << ";";
                }

                if (showTrack) {
                    result << "    ";

                    for(int j = 0; j < this->order; j++) {
                        result << std::bitset<8>(this->track[j + i * this->order]) << ";";
                    }
                }
                
                result << '\n';
            }

            return result.str();
        }

        bool Found() {
            return this->found;
        }

        uint8_t* GetCayley() {
            return this->cayley;
        }
};
