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
 * @brief Find quasigroups, which might lack the associative property.
 * This algorithm systematically (not randomly) finds latin squares
 * of specific orders. (The latin square is the Cayley table of the
 * quasigroup or a group.)
 */
class LatinHeuristics {
    private:
        int order;
        int size;
        uint8_t *cayley;   // Cayley table
        uint32_t *track;    // Bitmap of already used values.
        uint32_t *rows;     // Bitmap of currently used values in rows.
        uint32_t *columns;  // Bitmap of currently used values in columns.
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
            uint32_t bit = ((uint32_t)1) << (value - 1);

            this->cayley[this->pos] = value;
            this->track[this->pos] |= bit;
            this->rows[this->y] |= bit;
            this->columns[this->x] |= bit;

            /*std::cout << "Columns[4]: " << std::bitset<32>(this->columns[4]) << '\n';
            std::cout << "Rows[1]: " << std::bitset<32>(this->rows[1]) << '\n';
            std::cout << "Track[9]: " << std::bitset<32>(this->track[9]) << '\n';
            */
            //std::cout << this->GetAsText() << '\n';
        }

        inline void Unset(bool isBackTracking) {
            if (isBackTracking) {
                this->track[this->pos] = 0;
            }

            uint8_t value = this->cayley[this->pos];
            if (value == 0) {
                // Nothing to unset
                return;
            }

            uint32_t bit = ~(((uint32_t)1) << (value - 1));

            this->cayley[this->pos] = 0;
            this->rows[this->y] &= bit;
            this->columns[this->x] &= bit;
        }

        inline int FindPossibleValue() {
            uint32_t bitMap = ~(this->track[this->pos] | this->rows[this->y] | this->columns[this->x]);
            
            // https://gcc.gnu.org/onlinedocs/gcc-4.8.0/gcc/Other-Builtins.html
            return __builtin_ffs(bitMap);
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
        LatinHeuristics(uint8_t order) {
            this->order = order;
            this->size = order * order;
            this->cayley = new uint8_t[this->size];
            this->track = new uint32_t[this->size];
            this->rows = new uint32_t[order];
            this->columns = new uint32_t[order];
            this->x = 1;
            this->y = 1;
            this->pos = this->order + 1;
            this->found = false;

            memset(this->cayley, 0, this->size * sizeof(uint8_t));
            memset(this->track, 0, this->size * sizeof(uint32_t));
            memset(this->rows, 0, order * sizeof(uint32_t));
            memset(this->columns, 0, order * sizeof(uint32_t));

            /*
                Fixed values
            */
            for(int i = 0; i < this->order; i++) {
                /*
                    Horizontal
                */
                *(this->cayley + i) = i + 1;
                this->columns[i] |= 1 << i;

                /*
                    Vertical
                */
                *(this->cayley + i * this->order) = i + 1;
                this->rows[i] |= 1 << i;
            }
        }

        ~LatinHeuristics() {
            delete[] this->cayley;
            delete[] this->track;
            delete[] this->rows;
            delete[] this->columns;
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

                //std::cin.get();

            } while(this->StepForward());

            this->found = true;

            if (next > this->order) {
                // Last one
                return false;
            }

            // More to be expected
            return true;
        }

        std::string GetAsText() {
            std::stringstream result;

            for(int i = 0; i < this->order; i++) {
                for(int j = 0; j < this->order; j++) {
                    result << std::setfill('0') << std::setw(2)
                        << (int)(*(this->cayley + j + i * this->order)) << ";";
                }

                result << '\n';
            }

            return result.str();
        }

        bool Found() {
            return this->found;
        }
};
