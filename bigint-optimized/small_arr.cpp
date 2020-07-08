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
    for (size_t i = 0; i < size; i++) {
        arr[i] = other.arr[i];
    }
}

