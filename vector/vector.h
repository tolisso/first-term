#ifndef VECTOR_H
#define VECTOR_H

#include <cstdio>
#include <algorithm>
#include <assert.h>
#include <iostream>

template <typename T>
struct vector
{
    typedef T* iterator;
    typedef T const* const_iterator;

    vector() noexcept { // O(1) nothrow
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    };


    vector(vector const& other) : vector() { // O(N) strong
        if (other.capacity_ == 0) {
            size_ = 0;
            capacity_ = 0;
            data_ = nullptr;
            return;
        }
        size_t new_size = other.size_;
        size_t new_capacity = other.size_;
        size_t i = 0;
        T* new_data;
        try {
            new_data = static_cast<T*>(operator new(new_size * sizeof(T)));
            for (; i < new_size; i++) {
                new(new_data + i) T(other[i]);
            }
        } catch (std::exception const& exc) {
            delete_arr(new_data, i);
            throw;
        }
        delete_arr(data_, size_);
        data_ = new_data;
        size_ = new_size;
        capacity_ = new_capacity;
    }

    vector& operator=(vector const& other) { // O(N) strong
        vector<T> temp(other);
        swap(temp);
        return *this;
    }

    ~vector() noexcept { // O(N) nothrow
        delete_arr(data_, size_);
    }

    T& operator[](size_t i) noexcept { // O(1) nothrow
        return data_[i];
    }
    T const& operator[](size_t i) const noexcept { // O(1) nothrow
        return data_[i];
    }

    T* data() noexcept { // O(1) nothrow
        return data_;
    };
    T const* data() const noexcept { // O(1) nothrow
        return data_;
    }
    size_t size() const noexcept { // O(1) nothrow
        assert (size_ >= 0);
        return size_;
    }

    T& front() noexcept { // O(1) nothrow
        assert (size_ >= 0);
        return data_[0];
    }
    T const& front() const noexcept { // O(1) nothrow
        assert (size_ > 0);
        return data_[0];
    };

    T& back() noexcept { // O(1) nothrow
        assert (size_ > 0);
        return data_[size_ - 1];
    }
    T const& back() const noexcept { // O(1) nothrow
        assert (size_ > 0);
        return data_[size_ - 1];
    }
    void push_back(T const& elem) { // O(1)* strong
        if (size_ == capacity_) {
            T temp(elem);
            reserve(capacity_ * 2 + 1);
            new(data_ + size_) T(temp);
            size_++;
        } else {
            new(data_ + size_) T(elem);
            size_++;
        }
    }
    void pop_back() noexcept { // O(1) nothrow
        assert(size_ != 0);
        data_[--size_].~T();
    }

    bool empty() const noexcept { // O(1) nothrow
        return size_ == 0;
    }

    size_t capacity() const noexcept { // O(1) nothrow
        return capacity_;
    };
    void reserve(size_t new_capacity) { // O(N) strong
        if (new_capacity <= capacity_) {
            return;
        }
        new_buffer(new_capacity);
    }
    void shrink_to_fit() { // O(N) strong
        if (size_ == 0) {
            new_buffer(0);
        } else if (capacity_ != size_)
            new_buffer(size_);
    }

    void clear() noexcept { // O(N) nothrow
        for (; size_ != 0;) {
            pop_back();
        }
    };

    void swap(vector& other) noexcept { // O(1) nothrow
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

    iterator begin() noexcept { // O(1) nothrow
        return data_;
    }
    iterator end() noexcept { // O(1) nothrow
        return data_ + size_;
    }

    const_iterator begin() const noexcept { // O(1) nothrow
        return data_;
    };
    const_iterator end() const noexcept { // O(1) nothrow
        return data_ + size_;
    };

    iterator insert(iterator pos_it, T const& elem) { // O(N) weak
        size_t pos = pos_it - begin();
        push_back(elem);
        size_t i = size_ - 1;
        for (; i > pos; i--) {
            std::swap(data_[i], data_[i - 1]);
        }
        return data_ + i;
    }
    iterator insert(const_iterator pos, T const& elem) { // O(N) weak
        return insert(const_cast<iterator>(pos), elem);
    };

    iterator erase(iterator pos_it) { // O(N) weak
        size_t pos = pos_it - begin();
        size_t i = pos;
        for (; i + 1 < size_; i++) {
            std::swap(data_[i], data_[i + 1]);
        }
        pop_back();
        return begin() + pos;
    }
    iterator erase(const_iterator pos) { // O(N) weak
        return erase(const_cast<iterator >(pos));
    }

    iterator erase(iterator first, iterator last) { // O(N) weak
        for (iterator l = first, r = last; r != end(); l++, r++) {
            std::swap(*l, *r);
        }
        for (size_t i = 0; i < last - first; i++) {
            pop_back();
        }
        return data_ + (last - first);
    }
    void new_buffer(size_t new_capacity) {
        T* new_data;
        size_t i = 0;
        try {
            new_data = static_cast<T*>(operator new(new_capacity * sizeof(T)));
            for (; i < size_; i++) {
                new(new_data + i) T(data_[i]);
            }
        } catch (std::exception const& exc) {
            delete_arr(new_data, i);
            throw;
        }
        for (size_t i = size_; i > 0; i--) {
            data_[i - 1].~T();
        }
        if (capacity_ != 0) {
            operator delete(data_);
        }
        if (new_capacity == 0) {
            operator delete(new_data);
            data_ = nullptr;
            capacity_ = 0;
        } else {
            data_ = new_data;
            capacity_ = new_capacity;
        }
    }
    void delete_arr(T* arr, size_t sz) {
        for (size_t j = sz; j > 0; j--) {
            arr[j - 1].~T();
        }
        operator delete(arr);
    }

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};

#endif // VECTOR_H
