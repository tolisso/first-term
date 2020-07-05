#include "array_.h"

array_::array_(size_t size) {
    if (size > 4) {
        big.arr = new std::vector<uint32_t>();
        big.arr->resize(size);
        big.refs = new uint32_t(1);
        is_big = true;
    } else {
        is_big = false;
    }
    size_ = size;
}

array_::array_(array_ const& other) {
    if (other.is_big) {
        big.arr = other.big.arr;
        big.refs = other.big.refs;
        (*big.refs)++;
    } else {
        small.arr.equal(other.small.arr, other.size_);
    }
    is_big = other.is_big;
    size_ = other.size_;
}

array_& array_::operator=(array_ const& other) {
    clear_big();
    if (other.is_big) {
        big.arr = other.big.arr;
        big.refs = other.big.refs;
        (*other.big.refs)++;
    } else {
        small.arr.equal(other.small.arr, other.size_);
    }
    is_big = other.is_big;
    size_ = other.size_;
    return *this;
}

array_::~array_() {
    clear_big();
}

void array_::clear_big() {
    if (is_big) {
        if (--(*big.refs) == 0) {
            delete big.arr;
            delete big.refs;
        }
    }
}

void array_::set(size_t i, uint32_t val) {
    if (!is_big) {
        small.arr[i] = val;
    } else {
        if (*big.refs != 1) {
//        std::cout << *refs << std::endl;
            auto new_arr = new std::vector<uint32_t >();
            new_arr->resize(size_);
            for (size_t i = 0; i < size_; i++) {
                (*new_arr)[i] = (*big.arr)[i];
            }
            clear_big();
            big.refs = new uint32_t(1);
            big.arr = new_arr;
        }
        (*big.arr)[i] = val;
    }
}

uint32_t array_::get(size_t i) const {
    if (!is_big) {
        return small.arr[i];
    } else {
        return (*big.arr)[i];
    }
}

void array_::to_big() {
    if (!is_big) {
        auto new_arr = new std::vector<uint32_t>();
        for (size_t i = 0; i < size_; i++) {
            new_arr->push_back(small.arr[i]);
        }
        is_big = true;
        big.arr = new_arr;
        big.refs = new uint32_t(1);
    }
}

void array_::make_unique() { /// Pre: is_big == true;
    if (*big.refs != 1) {
        auto new_arr = new std::vector<uint32_t >();
        new_arr->resize(size_);
        for (size_t i = 0; i < size_; i++) {
            (*new_arr)[i] = (*big.arr)[i];
        }
        clear_big();
        big.refs = new uint32_t(1);
        big.arr = new_arr;
    }
}

void array_::pop_back() {
    if (is_big) {
        make_unique();
        big.arr->pop_back();
    }
    size_--;
}

void array_::push_back(uint32_t i) {
    if (is_big) {
        make_unique();
        big.arr->push_back(i);
        size_++;
    } else { // is small
        if (size_ == 4) {
            to_big();
            big.arr->push_back(i);
            size_++;
        } else {
            small.arr[size_++] = i;
        }
    }
}

void array_::resize(size_t size) {
    to_big();
    make_unique();
    big.arr->resize(size);
    size_ = size;
}

size_t array_::size() const {
    return size_;
}

bool array_::operator==(array_ const& other) const {
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

void array_::erase(size_t a, size_t b) {
    to_big();
    make_unique();
    big.arr->erase(big.arr->begin() + a, big.arr->begin() + b);
    size_ = big.arr->size();
}