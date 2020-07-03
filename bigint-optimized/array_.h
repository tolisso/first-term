//
// Created by tolisso on 7/3/20.
//

#ifndef BIGINT_ARRAY__H
#define BIGINT_ARRAY__H


#include <vector>
#include "small_arr.h"

struct big_ {
    uint32_t *refs;
    std::vector<uint32_t> *arr;
};

struct small_ {
    small_arr arr;
};

struct array_ {
public:
    explicit array_(size_t size);
    array_(array_ const&);
//    uint32_t& operator[](size_t);
//    uint32_t const& operator[](size_t) const;
    void set(size_t, uint32_t);
    bool operator==(array_ const&) const;
    uint32_t get(size_t) const;
    array_& operator=(array_ const&);
    ~array_();
    void clear_big();
    size_t size() const;
    void pop_back();
    void push_back(uint32_t);
    void resize(size_t);
    void erase(size_t, size_t);
private:
    void to_big();
    void make_unique();
    size_t size_;
    bool is_big;
    union {
        big_ big;
    };
};

#endif //BIGINT_ARRAY__H
