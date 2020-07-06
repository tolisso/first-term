//
// Created by tolisso on 7/3/20.
//

#ifndef BIGINT_SMALL_ARR_H
#define BIGINT_SMALL_ARR_H

#include <cstdint>
#include <cstddef>

struct small_arr {
    uint32_t arr[4];
    uint32_t const& operator[](size_t) const;
    uint32_t& operator[](size_t);
    void equate(small_arr const&, size_t const size);
};

#endif //BIGINT_SMALL_ARR_H
