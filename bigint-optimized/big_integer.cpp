#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <iostream>

#ifdef DEBUG
#define DEBUG_IF(x) if(x)
#else
#define DEBUG_IF(x) if(false)
#endif

big_integer::big_integer() : arr(1) {
    arr.set(0, 0);
    sign = false;
}
big_integer::~big_integer() = default;

big_integer::big_integer(big_integer const &other) : arr(other.arr) {
    sign = other.sign;
}

big_integer::big_integer(int a) : arr(1){
    int64_t tmp = a;
    arr.set(0, static_cast<uint32_t>(std::abs(tmp)));
    sign = a < 0;
}

big_integer& big_integer::operator=(big_integer const& other) = default;

big_integer::big_integer(std::string const &str) : arr(1) {
    sign = false;
    arr.set(0, 0);
    bool result_sign;
    size_t i = 0;
    if (str[0] == '-') {
        i++;
        result_sign = true;
    } else {
        result_sign = false;
    }
    arr.push_back(0);
    for (; i != str.length(); i++) {
        mul_uint(10);
        *this += str[i] - '0';
    }
    sign = result_sign;
}

big_integer binary_func(big_integer const& f, big_integer const& s, std::function<uint32_t(uint32_t, uint32_t)> func) {
    big_integer a = f.twos_complement();
    big_integer b = s.twos_complement();
    if (a.arr.size() < b.arr.size()) {
        std::swap(a, b);
    }
    size_t b_size = b.arr.size();
    b.arr.resize(a.arr.size());
    uint32_t val;
    val = (b.arr.get(b_size - 1) >> 31) == 0 ? 0 : UINT32_MAX;
    for (size_t i = b_size; i < b.arr.size(); i++) {
        b.arr.set(i, val);
    }
    big_integer c(a);
    for (size_t i = 0; i < a.arr.size(); i++) {
        c.arr.set(i, func(a.arr.get(i), b.arr.get(i)));
    }
    return c.basic_binary_represent().zero_abs();
}

big_integer big_integer::operator-() const {
    big_integer ans(*this);
    ans.sign = !ans.sign;
    return ans.zero_abs();
}

big_integer& big_integer::operator+=(big_integer const &other) {
    if (is_zero(other)) {
        return *this;
    }
    if (sign ^ other.sign) {
        return (*this -= -other);
    }
    uint32_t remainder = 0;
    for (size_t i = 0; remainder != 0 || i < arr.size() || i < other.arr.size(); i++) {
        if (i == arr.size()) {
            arr.push_back(0);
        }
        uint32_t left = 0;
        uint32_t right = 0;
        if (i < arr.size()) {
            left = arr.get(i);
        }
        if (i < other.arr.size()) {
            right = other.arr.get(i);
        }
        arr.set(i, left + right + remainder);
        remainder = (static_cast<uint64_t>(left) + right + remainder) >> 32;
    }
    return strip().zero_abs();
}

big_integer& big_integer::operator-=(big_integer const &other) {
    if (is_zero(other)) {
        return *this;
    }
    if (sign ^ other.sign) {
        return (*this += -other);
    }
    uint32_t remainder = 0;
    for (size_t i = 0; i < arr.size() || i < other.arr.size(); i++) {
        if (i == arr.size()) {
            arr.push_back(0);
        }
        uint32_t left = 0;
        uint32_t right = 0;
        if (i < arr.size()) {
            left = arr.get(i);
        }
        if (i < other.arr.size()) {
            right = other.arr.get(i);
        }
        arr.set(i, left - right - remainder);
        remainder = right + remainder <= left ? 0 : 1;
    }
    strip();
    if (remainder == 1) {
        big_integer ans(0);
        ans.arr.resize(arr.size() + 1);
        ans.arr.set(arr.size(), 1);
        sign = !sign;
        ans.sign = sign;
        *this = ans -= *this;
    }
    return strip().zero_abs();
}

big_integer& big_integer::operator*=(big_integer const &other) {
    big_integer a = this->abs();
    big_integer b = other.abs();
    big_integer ans;
    ans.sign = false;
    ans.arr.resize(a.arr.size() + b.arr.size() + 2);
    for (size_t i = 0; i < a.arr.size(); i++) {
        uint64_t remainder = 0;
        for (size_t j = 0; j < b.arr.size(); j++) {
            uint64_t new_val = remainder + ans.arr.get(i + j) + static_cast<uint64_t >(a.arr.get(i)) * b.arr.get(j);
            remainder = new_val >> 32;
            ans.arr.set(i + j, new_val);
        }
        for (size_t j = i + b.arr.size(); remainder != 0; j++) {
            uint64_t new_val = remainder + ans.arr.get(j);
            remainder = new_val >> 32;
            ans.arr.set(j, new_val);
        }
    }
    ans.sign = this->sign != other.sign;
    return *this = ans.strip().zero_abs();
}

big_integer& big_integer::operator/=(big_integer const &val) {
    big_integer r = this->abs();
    big_integer d = val.abs();
    if (r < d) {
        return *this = big_integer::ZERO;
    }
    if (d.arr.size() == 1) {
        r.div_uint(d.arr.get(0));
        r.sign = this->sign != val.sign;
        return *this = r;
    }
    uint32_t f = (static_cast<uint64_t >(1) << 32) / (static_cast<uint64_t >(d.arr.get(d.arr.size() - 1)) + 1);
    r.mul_uint(f);
    d.mul_uint(f);
    r.arr.push_back(0);
    big_integer q(0);
    size_t q_size = r.arr.size();
    q.arr.resize(q_size);
    size_t n = r.arr.size();
    size_t m = d.arr.size();
    for (size_t k = n - m - 1; ; k--) {
        /// if your compilator hasn't got __int128 please use gmp mpz_t, logics still the same,
        /// because r3, d2 not more than 2^96
        __uint128_t r3 = 0;
        __uint128_t d2 = 0;
        r3 += r.arr.get(m + k);
        r3 <<= 32;
        r3 += r.arr.get(m + k - 1);
        r3 <<= 32;
        r3 += r.arr.get(m + k - 2);
        d2 += d.arr.get(m - 1);
        d2 <<= 32;
        d2 += d.arr.get(m - 2);
        uint32_t qt = std::min(static_cast<uint32_t>(r3 / d2), UINT32_MAX);
        big_integer dq(d);
        dq.mul_uint(qt);
        if (compare(dq, r, k, m)) {
            qt--;
            dq -= d;
        }
        q.arr.set(k, qt);
        difference(dq, r, k, m);
        if (r.arr.get(r.arr.size() - 1) == 0) {
            r.arr.pop_back();
        }
        if (k == 0) {
            break;
        }
    }
    q.sign = this->sign != val.sign;
    return *this = q.strip().zero_abs();
}


bool compare (big_integer const& dq, big_integer const& r, size_t k, size_t m) {
    for (size_t i = 0; i <= m; i++) {
        uint32_t cur_dq = 0;
        if (m - i < dq.arr.size()) {
            cur_dq = dq.arr.get(m - i);
        }
        if (r.arr.get(m + k - i) != cur_dq) {
            return r.arr.get(m + k - i) < cur_dq;
        }
    }
    return false;
}

void difference(big_integer const& dq, big_integer &r, size_t k, size_t m) {
    uint32_t remainder = 0;
    size_t from = k;
    for (size_t i = 0; i <= m; i++) {
        uint32_t cur_dq = 0;
        if (i < dq.arr.size()) {
            cur_dq = dq.arr.get(i);
        }
        uint64_t val = (static_cast<uint64_t>(r.arr.get(from + i)) - cur_dq - remainder);
        remainder = (r.arr.get(from + i) < cur_dq + remainder);
        r.arr.set(from + i, static_cast<uint32_t >(val));
    }
}

big_integer& big_integer::mul_uint(uint32_t val) {
    if (val == 0) {
        *this = big_integer::ZERO;
    }
    uint64_t remainder = 0;
    for (size_t i = 0; i < arr.size(); i++) {
        uint64_t cur_num = (static_cast<uint64_t>(arr.get(i)) * val + remainder);
        arr.set(i, static_cast<uint32_t>(cur_num));
        remainder = (cur_num >> 32);
    }
    while (remainder != 0) {
        arr.push_back(static_cast<uint32_t>(remainder));
        remainder >>= 32;
    }
    return strip().zero_abs();
}

bool is_zero(big_integer const &a) {
    DEBUG_IF(a.arr.size() == 1 && a.arr.get(0) == 0 && a.sign) {
        throw "0 with sign \"-\"";
    }
    return a.arr.size() == 1 && a.arr.get(0) == 0;
}

bool comp_by_mod(big_integer const& a, big_integer const& b) {
    bool mod_ans;
    if (a.arr.size() < b.arr.size()) {
        mod_ans = true;
    } else if(a.arr.size() > b.arr.size()) {
        mod_ans = false;
    } else {
        for (size_t i = a.arr.size() - 1;; i--) {
            if (a.arr.get(i) < b.arr.get(i)) {
                mod_ans = true;
                break;
            }
            if (a.arr.get(i) > b.arr.get(i)) {
                mod_ans = false;
                break;
            }
            if (i == 0) {
                mod_ans = false;
                break;
            }
        }
    }
    return mod_ans;
}

bool operator==(big_integer const &a, big_integer const &b) {
    return (is_zero(a) && is_zero(b)) ||
           (a.sign == b.sign && a.arr == b.arr);
}

bool operator<(big_integer const &a, big_integer const &b) {
    if (a == b) {
        return false;
    }
    if (a.sign < b.sign) {
        return false;
    }
    if (a.sign > b.sign) {
        return true;
    }
    if (a.sign == false) { // && b.sign == 0
        return comp_by_mod(a, b);
    } else { // a.sign == 1 && b.sign == 1
        return comp_by_mod(b, a);
    }
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator>(big_integer const &a, big_integer const &b) {
    return b < a;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return  a < b || a == b;
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return b <= a;
}

big_integer big_integer::operator+() const {
    return big_integer(*this);
}

big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
    return a %= b;
}

big_integer& big_integer::operator%=(big_integer const &other) {
    big_integer divisor(*this);
    divisor /= other;
    divisor *= other;
    return *this -= divisor;
}

big_integer operator&(big_integer const& a, big_integer const& b) {
    return binary_func(a, b, [](uint32_t a, uint32_t b)->uint32_t {return a & b;});
}
big_integer& big_integer::operator&=(big_integer const& other) {
    return *this = *this & other;
}
big_integer operator|(big_integer const& a, big_integer const& b) {
    return binary_func(a, b, [](uint32_t a, uint32_t b)->uint32_t {return a | b;});
}
big_integer& big_integer::operator|=(big_integer const& other) {
    return *this = *this | other;
}
big_integer operator^(big_integer const& a, big_integer const& b) {
    return binary_func(a, b, [](uint32_t a, uint32_t b)->uint32_t {return a ^ b;});
}
big_integer& big_integer::operator^=(big_integer const& other) {
    return *this = *this ^ other;
}
big_integer big_integer::operator~() const {
    return binary_func(*this, big_integer::ZERO, [](uint32_t a, uint32_t b)->uint32_t {return ~a;});
}
void remove_pref(array_ &vec, size_t to) {
    vec.erase(0, to);
}
big_integer operator>>(big_integer const& a, int shift) {
    size_t discharge = shift / 32;
    if (a >= big_integer::ZERO) {
        big_integer ans = a;
        remove_pref(ans.arr, discharge);
        return ans.div_uint(static_cast<uint32_t > (1) << (shift % 32));
    } else {
        big_integer ans = a;
        big_integer shift_mod = 1;
        shift_mod <<= (shift);
        return (ans / shift_mod - 1).zero_abs();
    }
}
big_integer& big_integer::operator>>=(int shift) {
    return *this = (*this >> shift);
}
big_integer operator<<(big_integer const& a, int shift) {
    size_t discharge = shift / 32;
    big_integer ans = a;
    for (size_t i = 0; i < discharge; i++) {
        ans.arr.push_back(0);
    }
    for (size_t i = a.arr.size() - 1; ; i--) {
        uint32_t tmp = ans.arr.get(i);
        ans.arr.set(i, ans.arr.get(i + discharge));
        ans.arr.set(i + discharge, tmp);
        if (i == 0) {
            break;
        }
    }
    ans.sign = a.sign;
    return ans.mul_uint(static_cast<uint32_t > (1) << (shift % 32));
}
big_integer& big_integer::operator<<=(int shift) {
    return *this = (*this << shift);
}

uint32_t big_integer::mod_uint(uint32_t divisor) const {
    uint64_t remainder = 0;
    for (size_t i = arr.size() - 1; true; i--) {
        remainder <<= 32;
        remainder += arr.get(i);
        remainder %= divisor;
        if (i == 0) {
            break;
        }
    }
    return remainder;
}

big_integer& big_integer::div_uint(uint32_t divisor) {
    uint64_t remainder = 0;
    for (size_t i = arr.size() - 1; true; i--) {
        remainder <<= 32;
        remainder += arr.get(i);
        arr.set(i, remainder / divisor);
        remainder %= divisor;
        if (i == 0) {
            break;
        }
    }
    return strip().zero_abs();
}

big_integer& big_integer::strip() {
    for (size_t i = arr.size() - 1; i != 0; i--) {
        if (arr.get(i) == 0) {
            arr.pop_back();
        } else {
            break;
        }
    }
    return *this;
}

big_integer big_integer::twos_complement() const {
    big_integer ans(*this);
    if (!sign) {
        if ((arr.get(arr.size() - 1) >> 31) != 0) {
            ans.arr.push_back(0);
        }
        return ans;
    }
    ans.sign = false;
    big_integer zero(0);
    if (ans == zero) {
        return -*this;
    }
    if ((arr.get(arr.size() - 1) >> 31) != 0) {
        ans.arr.push_back(0);
        ans.arr.set(ans.arr.size() - 1, UINT32_MAX);
    }
    for (size_t i = 0; i < arr.size(); i++) {
        ans.arr.set(i, ans.arr.get(i) ^ UINT32_MAX);
    }
    return (ans + 1).zero_abs();
}
big_integer big_integer::basic_binary_represent() const {
    assert(!sign);
    big_integer ans(*this);
    ans.sign = (arr.get(arr.size() - 1) >> 31);

    if (ans.sign) {
        ans = ans + 1;
        for (size_t i = 0; i < arr.size(); i++) {
            ans.arr.set(i, ans.arr.get(i) ^ UINT32_MAX);
        }
    }
    return ans.strip().zero_abs();
}

std::string to_string(big_integer const& a) {
    std::stringstream ss;
    for (big_integer b(a); b.arr.size() != 1 || b.arr.get(0) != 0; b.div_uint(10)) {
        ss << b.mod_uint(10);
    }
    if (a.sign && !is_zero(a)) {
        ss << '-';
    } else if (is_zero(a)) {
        ss << 0;
    }
    std::string ans;
    ss >> ans;
    std::reverse(ans.begin(), ans.end());
    return ans;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    s << to_string(a);
    return s;
}

big_integer& big_integer::operator++() {
    return *this += 1;
}
big_integer big_integer::operator++(int) {
    big_integer ans(*this);
    *this += 1;
    return ans;
}
big_integer& big_integer::operator--() {
    return *this -= 1;
}
big_integer big_integer::operator--(int) {
    big_integer ans(*this);
    *this -= 1;
    return ans;
}

big_integer big_integer::abs() const {
    big_integer ans(*this);
    ans.sign = false;
    return ans;
}

big_integer& big_integer::zero_abs() {
    if (arr.size() == 1 && arr.get(0) == 0) {
        sign = false;
    }
    return *this;
}

big_integer big_integer::ZERO = 0;