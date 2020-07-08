//
// Created by tolisso on 7/3/20.
//

#ifndef BIGINT_UI32ARRAY_H
#define BIGINT_UI32ARRAY_H


#include <vector>
#include "small_arr.h"

struct big_arr {
    uint32_t refs;
    std::vector<uint32_t> arr;
};

struct ui32array {
public:
    explicit ui32array(size_t size);
    ui32array(ui32array const&);
    void set(size_t, uint32_t);
    bool operator==(ui32array const&) const;
    uint32_t get(size_t) const;
    ui32array& operator=(ui32array const&);
    ~ui32array();
    void soft_clear_big();
    size_t size() const;
    void pop_back();
    void push_back(uint32_t);
    void resize(size_t);
    void erase(size_t, size_t);
private:
    void to_big();
    void make_unique();

    static const size_t SMALL_MAX_SIZE = 4;

    size_t size_;
    bool is_big;
    union {
        big_arr* big;
        small_arr small;
    };
};

#endif //BIGINT_UI32ARRAY_H
