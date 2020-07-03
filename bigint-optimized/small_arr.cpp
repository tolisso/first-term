//
// Created by tolisso on 7/3/20.
//

#include "small_arr.h"

uint32_t& small_arr::operator[](size_t i) {
    if (i == 0) {
        return _0;
    } else if (i == 1) {
        return _1;
    } else if (i == 2) {
        return _2;
    }
    return _3;
}

uint32_t const& small_arr::operator[](size_t i) const {
    if (i == 0) {
        return _0;
    } else if (i == 1) {
        return _1;
    } else if (i == 2) {
        return _2;
    }
    return _3;
}
void small_arr::equal(small_arr const & other, size_t const&size) {
    switch(size) {
        case 4:
            _3 = other._3;
        case 3:
            _2 = other._2;
        case 2:
            _1 = other._1;
        case 1:
            _0 = other._0;
    }
}

