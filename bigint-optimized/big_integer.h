#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <gmp.h>
#include <iosfwd>
#include <stdint-gcc.h>
#include <vector>
#include <functional>

struct big_ {
    uint32_t *refs;
    std::vector<uint32_t> *arr;
};

struct small_arr {
    uint32_t _0;
    uint32_t _1;
    uint32_t _2;
    uint32_t _3;
    uint32_t const& operator[](size_t) const;
    uint32_t& operator[](size_t);
    void equal(small_arr const&, size_t const& size);
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
        small_ small;
    };
};

struct big_integer
{
    big_integer();
    big_integer(big_integer const& other);
    big_integer(int a);
    explicit big_integer(std::string const& str);
    ~big_integer();
    big_integer& operator=(big_integer const& other);

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(int rhs);
    big_integer& operator>>=(int rhs);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend big_integer operator&(big_integer const& a, big_integer const& b);
    friend big_integer operator|(big_integer const& a, big_integer const& b);
    friend big_integer operator^(big_integer const& a, big_integer const& b);

    friend big_integer operator<<(big_integer const& a, int b);
    friend big_integer operator>>(big_integer const& a, int b);

    friend std::string to_string(big_integer const& a);




private:
    array_ arr;
    bool sign;

    friend bool is_zero(big_integer const& a);
    friend bool comp_by_mod(big_integer const& a, big_integer const& b);
    big_integer& mul_uint(uint32_t val);
    uint32_t mod_uint(uint32_t divisor) const;
    big_integer& div_uint(uint32_t divisor);
    big_integer& strip();
    friend bool compare(big_integer const& dq, big_integer const& r, size_t k, size_t m);
    friend void difference(big_integer const& dq, big_integer &r, size_t k, size_t m);
    big_integer unsigned_binary() const;
    big_integer signed_binary() const;
    friend big_integer binary_func(big_integer a, big_integer b, std::function<uint32_t(uint32_t, uint32_t)> func);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);



bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif // BIG_INTEGER_H
