#pragma once
#include <cstddef>		// std::size_t
#include <ostream>		// std::basic_ostream
#include <stdexcept>	// std::out_of_range

// ちょっとエッチなconstexprバージョンの文字列
template<class Char>
class cstring {
	const Char* s_;
	const std::size_t n_;
public:
	constexpr cstring(const Char* s, std::size_t n) : s_{ s }, n_{ n } { /*if (s[n] != 0) throw std::invalid_argument("s is not null terminated.");*/ }
	template<std::size_t N>
	constexpr cstring(const Char(&s)[N]) : cstring{ s, N - 1 } {}
	// 文字列の指定したインデックス位置の文字を返します。
	constexpr Char operator[](std::size_t i) const { return i < n_ ? s_[i] : throw std::out_of_range("invalid index."); }
	// Cスタイルのnullで終わる文字列を返します。
	constexpr const Char* c_str() const noexcept { return s_; }
	// 文字列の要素数を返します。
	constexpr std::size_t size() const noexcept { return n_; }
	// 文字列が一致するか比較します。
	constexpr bool equal(const cstring<Char>& r) const noexcept { return size() == r.size() && equal(r, 0); }
private:
	constexpr bool equal(const cstring<Char>& r, std::size_t i) const noexcept { return i == size() || ((*this)[i] == r[i] && equal(r, i + 1)); }
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
