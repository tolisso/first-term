#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <iostream>

big_integer::big_integer() : arr(1) {
    arr.set(0, 0);
    sign = false;
}
big_integer::~big_integer() = default;

big_integer::big_integer(big_integer const &other) : arr(other.arr) {
    sign = other.sign;
}

big_integer::big_integer(int a) : arr(1){
    if (a == INT32_MIN) {
        arr.set(0, static_cast<uint32_t>(1) << 31);
        sign = true;
    } else if (a > 0) {
        arr.set(0, a);
        sign = false;
    } else { // if(a < 0)
        arr.set(0, -a);
        sign = true;
    }
}

big_integer& big_integer::operator=(big_integer const& other) {
    arr = other.arr;
    sign = other.sign;
    return *this;
}

big_integer::big_integer(std::string const &str) : arr(1) {
    sign = false;
    arr.set(0, 0);
    bool result_sign;
    std::string str_number = str;
    if (str_number[0] == '-') {
        str_number = str_number.substr(1, str_number.length() - 1);
        result_sign = true;
    } else {
        result_sign = false;
    }
    arr.push_back(0);
    for (size_t i = 0; i != str_number.length(); i++) {
        mul_uint(10);
        *this += str_number[i] - '0';
    }
    sign = result_sign;
}

big_integer binary_func(big_integer a, big_integer b, std::function<uint32_t(uint32_t, uint32_t)> func) {
    a = a.signed_binary();
    b = b.signed_binary();
    if (a.arr.size() < b.arr.size()) {
        big_integer c = a;
        a = b;
        b = c;
    }
    size_t b_size = b.arr.size();
    b.arr.resize(a.arr.size());
    uint32_t val;
    if ((b.arr.get(b_size - 1) >> 31) == 0) {
        val = 0;
    } else {
        val = UINT32_MAX;
    }
    for (size_t i = b_size; i < b.arr.size(); i++) {
        b.arr.set(i, val);
    }
    big_integer c(a);
    for (size_t i = 0; i < a.arr.size(); i++) {
        c.arr.set(i, func(a.arr.get(i), b.arr.get(i)));
    }
    return c.unsigned_binary();
}

big_integer big_integer::operator-() const {
    big_integer ans(*this);
    ans.sign = ans.sign ^ 1;
    return ans;
}

big_integer& big_integer::operator+=(big_integer const &other) {
    if ((sign ^ other.sign) == 1) {
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
    return strip();
}

big_integer& big_integer::operator-=(big_integer const &other) { /// undone
    if ((sign ^ other.sign) == 1) {
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
        if (right + remainder <= left) {
            remainder = 0;
        } else {
            remainder = 1;
        }
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
    return strip();
}

big_integer& big_integer::operator*=(big_integer const &other) {
    big_integer a = *this;
    big_integer b = other;
    a.sign = false;
    b.sign = false;
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
    return *this = ans.strip();
}

big_integer& big_integer::operator/=(big_integer const &val) {
    big_integer r(*this);
    big_integer d(val);
    r.sign = false;
    d.sign = false;
    if (r < d) {
        return *this = 0;
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
        __int128 r3 = 0;
        __int128 d2 = 0;
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
//        for (size_t i = 0; i < r.arr.size(); i++) {
//            std::cout << r.arr[i] << " ";
//        }
//        std::cout << std::endl;
//        for (size_t i = 0; i < dq.arr.size(); i++) {
//            std::cout << dq.arr[i] << " ";
//        }
//        std::cout << std::endl;
        difference(dq, r, k, m);
        if (r.arr.get(r.arr.size() - 1) == 0) {
            r.arr.pop_back();
        }
        if (k == 0) {
            break;
        }
    }
    q.sign = this->sign != val.sign;
    return *this = q.strip();
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
//        std::cout << r.arr[from + i] << std::endl;
//        std::cout << cur_dq << std::endl;
//        std::cout << std::endl;
        uint64_t val = (static_cast<uint64_t>(r.arr.get(from + i)) - cur_dq - remainder);
        remainder = (r.arr.get(from + i) < cur_dq + remainder);
        r.arr.set(from + i, static_cast<uint32_t >(val));
    }
}

big_integer& big_integer::mul_uint(uint32_t val) {
    if (val == 0) {
        arr = array_(1);
        arr.set(0, 0);
        return *this;
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
    return strip();
}

bool is_zero(big_integer const &a) {
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
    if (a.sign == 0) { // && b.sign == 0
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
    return binary_func(*this, 0, [](uint32_t a, uint32_t b)->uint32_t {return ~a;});
}
void remove_pref(array_ &vec, size_t to) {
    vec.erase(0, to);
}
big_integer operator>>(big_integer const& a, int shift) {
    size_t discharge = shift / 32;
    if (a >= 0) {
        big_integer ans = a;
        remove_pref(ans.arr, discharge);
        return ans.div_uint(static_cast<uint32_t > (1) << (shift % 32));
    } else {
        big_integer ans = a;
        big_integer shift_mod = 1;
        shift_mod <<= (shift);
        return ans / shift_mod - 1;
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
    return strip();
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

big_integer big_integer::signed_binary() const {
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
    return ans + 1;
}
big_integer big_integer::unsigned_binary() const {
    assert(!sign);
    big_integer ans(*this);
    ans.sign = (arr.get(arr.size() - 1) >> 31);

    if (ans.sign) {
        ans = ans + 1;
        for (size_t i = 0; i < arr.size(); i++) {
            ans.arr.set(i, ans.arr.get(i) ^ UINT32_MAX);
        }
    }
    return ans.strip();
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

///=========================@_array part@=========================

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

//uint32_t& array_::operator[](size_t i) {
//    if (size_ <= 4) {
//        return small.arr[i];
//    }
//    return big.arr[i];
//}
//
//uint32_t const& array_::operator[](size_t i) const {
//    if (size_ <= 4) {
//        return small.arr[i];
//    }
//    return big.arr[i];
//}

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
    to_big();
    make_unique();
    big.arr->pop_back();
    size_--;
}

void array_::push_back(uint32_t i) {
    to_big();
    make_unique();
    big.arr->push_back(i);
    size_++;
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

uint32_t& small_arr::operator[](size_t i) {
    if (i == 0) {
        return _0;
    } else if (i == 1) {
        return _1;
    } else if (i == 2) {
        return _2;
    }
    return _3;
}

uint32_t const& small_arr::operator[](size_t i) const {
    if (i == 0) {
        return _0;
    } else if (i == 1) {
        return _1;
    } else if (i == 2) {
        return _2;
    }
    return _3;
}
inline void small_arr::equal(small_arr const & other, size_t const&size) {
    switch(size) {
        case 4:
            _3 = other._3;
        case 3:
            _2 = other._2;
        case 2:
            _1 = other._1;
        case 1:
            _0 = other._0;
    }
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