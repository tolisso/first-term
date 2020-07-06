//
// Created by tolisso on 7/3/20.
//

#include "small_arr.h"

uint32_t& small_arr::operator[](size_t i) {
    return arr[i];
}

uint32_t const& small_arr::operator[](size_t i) const {
    return arr[i];
}
void small_arr::equate(small_arr const & other, size_t const size) {
    switch(size) {
        case 4:
            arr[3] = other.arr[3];
        case 3:
            arr[2] = other.arr[2];
        case 2:
            arr[1] = other.arr[1];
        case 1:
            arr[0] = other.arr[0];
    }
}

