//
// Created by tolisso on 7/3/20.
//

#ifndef BIGINT_SMALL_ARR_H
#define BIGINT_SMALL_ARR_H

#include <cstdint>
#include <cstddef>

struct small_arr {
    static const size_t MAX_SIZE = 4;
    uint32_t arr[MAX_SIZE];
    uint32_t const& operator[](size_t) const;
    uint32_t& operator[](size_t);
    void equate(small_arr const&, size_t const size);
};

#endif //BIGINT_SMALL_ARR_H
