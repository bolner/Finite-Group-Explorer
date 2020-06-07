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
#include <string>

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
                        this->message = "Not abelian. " + std::to_string(i + 1) + " * " + std::to_string(j + 1)
                            + " != " + std::to_string(j + 1) + " * " + std::to_string(i + 1);
                        return false;
                    }
                }
            }

            return true;
        }
};
