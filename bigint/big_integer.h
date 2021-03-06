#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <gmp.h>
#include <iosfwd>
#include <stdint-gcc.h>
#include <vector>
#include <functional>

struct big_integer
{
    big_integer();
    big_integer(big_integer const& other);
    big_integer(int a);
    explicit big_integer(std::string const& str);
    ~big_integer();
    big_integer& operator=(big_integer const& other);

    big_integer abs() const;
    big_integer& zero_abs();

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(size_t);
    big_integer& operator>>=(size_t);

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

    friend big_integer operator<<(big_integer const& a, size_t b);
    friend big_integer operator>>(big_integer const& a, size_t b);

    friend std::string to_string(big_integer const& a);

private:
    std::vector<uint32_t> arr;
    bool sign;
    static big_integer ZERO;

    friend bool is_zero(big_integer const& a);
    friend bool comp_by_mod(big_integer const& a, big_integer const& b);
    big_integer& mul_uint(uint32_t val);
    uint32_t mod_uint(uint32_t divisor) const;
    big_integer& div_uint(uint32_t divisor);
    big_integer& strip();
    friend bool compare(big_integer const& dq, big_integer const& r, size_t k, size_t m);
    friend void difference(big_integer const& dq, big_integer &r, size_t k, size_t m);
    big_integer basic_binary_represent() const;
    big_integer twos_complement() const;
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
