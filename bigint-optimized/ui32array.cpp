#include "ui32array.h"

ui32array::ui32array(size_t size) {
    if (size > SMALL_MAX_SIZE) {
        big->arr = std::vector<uint32_t>();
        big->arr.resize(size);
        big->refs = 1;
        is_big = true;
    } else {
        is_big = false;
        for (size_t i = 0; i < size; i++) {
            small[i] = 0;
        }
    }
    this->size_ = size;
}

ui32array::ui32array(ui32array const& other) {
    if (other.is_big) {
        big = other.big;
        big->refs++;
    } else {
        small.equate(other.small, other.size_);
    }
    is_big = other.is_big;
    size_ = other.size_;
}

ui32array& ui32array::operator=(ui32array const& other) {
    soft_clear_big();
    if (other.is_big) {
        big = other.big;
        big->refs++;
    } else {
        small.equate(other.small, other.size_);
    }
    is_big = other.is_big;
    size_ = other.size_;
    return *this;
}

ui32array::~ui32array() {
    soft_clear_big();
}

void ui32array::soft_clear_big() {
    if (is_big) {
        if (--(big->refs) == 0) {
            delete big;
        }
    }
}

void ui32array::set(size_t i, uint32_t val) {
    if (!is_big) {
        small[i] = val;
    } else {
        if (big->refs != 1) {
            big_arr *new_big = new big_arr();
            new_big->arr = std::vector<uint32_t >(big->arr);
            new_big->refs = 1;
            soft_clear_big();
            big = new_big;
        }
        big->arr[i] = val;
    }
}

uint32_t ui32array::get(size_t i) const {
    if (!is_big) {
        return small[i];
    } else {
        return big->arr[i];
    }
}

void ui32array::to_big() {
    if (!is_big) {
        big_arr *new_big = new big_arr();
        new_big->arr = std::vector<uint32_t>(small.arr, small.arr + size_);
        new_big->refs = 1;
        big = new_big;
        is_big = true;
    }
}

void ui32array::make_unique() { /// Pre: is_big == true;
    if (is_big && big->refs != 1) {
        big_arr *new_big = new big_arr();
        new_big->arr = big->arr;
        new_big->refs = 1;
        soft_clear_big();
        big = new_big;
    }
}

void ui32array::pop_back() {
    if (is_big) {
        make_unique();
        big->arr.pop_back();
    }
    size_--;
}

void ui32array::push_back(uint32_t i) {
    if (is_big) {
        make_unique();
        big->arr.push_back(i);
        size_++;
    } else { // is small
        if (size_ == SMALL_MAX_SIZE) {
            to_big();
            big->arr.push_back(i);
            size_++;
        } else {
            small[size_++] = i;
        }
    }
}

void ui32array::resize(size_t size) {
    if (size > SMALL_MAX_SIZE && !is_big) {
        to_big();
    }
    if (is_big) {
        make_unique();
        big->arr.resize(size);
        this->size_ = size;
    } else {
        for (size_t i = this->size_; i < size; i++) {
            small.arr[i] = 0;
        }
        this->size_ = size;
    }
}

size_t ui32array::size() const {
    return size_;
}

bool ui32array::operator==(ui32array const& other) const {
    if (size_ != other.size_) {
        return false;
    }
    for (size_t i = 0; i < size_; i++) {
        if (this->get(i) != other.get(i)) {
            return false;
        }
    }
    return true;
}

void ui32array::erase(size_t a, size_t b) {
    to_big();
    make_unique();
    big->arr.erase(big->arr.begin() + a, big->arr.begin() + b);
    size_ = big->arr.size();
}
