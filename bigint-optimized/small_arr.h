//
// Created by tolisso on 7/3/20.
//

#ifndef BIGINT_SMALL_ARR_H
#define BIGINT_SMALL_ARR_H

#include <stdint-gcc.h>
#include <cstddef>

struct small_arr {
    uint32_t _0;
    uint32_t _1;
    uint32_t _2;
    uint32_t _3;
    uint32_t const& operator[](size_t) const;
    uint32_t& operator[](size_t);
    void equal(small_arr const&, size_t const& size);
};

#endif //BIGINT_SMALL_ARR_H
