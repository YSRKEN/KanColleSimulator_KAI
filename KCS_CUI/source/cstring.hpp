#pragma once
#include <cstddef>		// std::size_t
#include <ostream>		// std::basic_ostream
#include <stdexcept>	// std::out_of_range
#include <limits>
// ちょっとエッチなconstexprバージョンの文字列
template<class Char>
class cstring {
public:
	typedef std::size_t size_type;
	static constexpr size_type npos = std::numeric_limits<size_type>::max();
private:
	const Char* s_;
	const size_type n_;
public:
	constexpr cstring(const Char* s, size_type n) : s_{ s }, n_{ n } { /*if (s[n] != 0) throw std::invalid_argument("s is not null terminated.");*/ }
	template<size_type N>
	constexpr cstring(const Char(&s)[N]) : cstring{ s, N - 1 } {}
	// 文字列の指定したインデックス位置の文字を返します。
	constexpr Char operator[](size_type i) const { return i < n_ ? s_[i] : throw std::out_of_range("invalid index."); }
	// Cスタイルのnullで終わる文字列を返します。
	constexpr const Char* c_str() const noexcept { return s_; }
	// 文字列の要素数を返します。
	constexpr size_type size() const noexcept { return n_; }
	// 文字列が一致するか比較します。
	constexpr bool equal(const cstring<Char>& r) const noexcept { return size() == r.size() && equal(r, 0); }
	constexpr size_type find(Char ch, size_type i = 0) const noexcept { return i == size() ? npos : s_[i] == ch ? i : find(ch, i + 1); }
	constexpr size_type find(const Char* str, size_type i = 0) const noexcept { return i == size() ? npos : startWith(str, i) ? i : find(str, i + 1); }
private:
	constexpr bool equal(const cstring<Char>& r, size_type i) const noexcept { return i == size() || ((*this)[i] == r[i] && equal(r, i + 1)); }
	constexpr bool startWith(const Char* str, size_type i) const noexcept { return *str == 0 || (i < size() && s_[i] == *str && startWith(str + 1, i + 1)); }
};

template<class Char>
constexpr bool operator==(const cstring<Char>& l, const cstring<Char>& r) noexcept { return l.equal(r); }
template<class Char>
constexpr bool operator!=(const cstring<Char>& l, const cstring<Char>& r) noexcept { return !(l == r); }

template<class Char, class CharTraits>
std::basic_ostream<Char, CharTraits>& operator<<(std::basic_ostream<Char, CharTraits>& os, const cstring<Char>& str) { return os << str.c_str(); }

constexpr cstring<char> operator "" _cs(const char* s, std::size_t n) { return{ s, n }; }
constexpr cstring<wchar_t> operator "" _cs(const wchar_t* s, std::size_t n) { return{ s, n }; }
constexpr cstring<char16_t> operator "" _cs(const char16_t* s, std::size_t n) { return{ s, n }; }
constexpr cstring<char32_t> operator "" _cs(const char32_t* s, std::size_t n) { return{ s, n }; }
