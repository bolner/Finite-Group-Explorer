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
#include <vector>

/**
 * @brief Returns all combinations of length k from n elements.
 */
class Combinator {
    private:
        int n, k, i, j;
        std::vector<uint8_t> combination;
    
    public:
        /**
         * @brief Construct a new Combinator object
         * 
         * @param n The size of the set
         * @param k Lengths of the tuples to be chosen in all combinations.
         */
        Combinator(int n, int k) : n(n), k(k), i(0), combination(k, 0) { }

        /**
         * @brief Get the next combination.
         * 
         * @param output Puts the result into this vector. The values are
         * 0-based index values.
         * @return Returns true if the output parameter has been assigned
         * to a new combination. Returns false if there are no new
         * combinations and the output is unchanged.
         */
        bool Next(std::vector<uint8_t> &output) {
            if (i < 0) {
                return false;
            }

            for (j = i + 1; j < k; j++) {
                combination[j] = combination[j - 1] + 1;
            }

            i = j - 1;
            output = combination;

            while (combination[(i + k) % k] == i + n - k) {
                --i;
            }
            
            combination[(i + k) % k]++;

            return true;
        }
};
