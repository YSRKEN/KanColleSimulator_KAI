/*=============================================================================
Copyright (C) 2016 yumetodo

Distributed under the Boost Software License, Version 1.0.
(See http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef ARITHMETIC_CONVERT_INC_ARITHMETIC_CONVERT_HPP_
#define ARITHMETIC_CONVERT_INC_ARITHMETIC_CONVERT_HPP_
#include <codecvt>
#include <limits>
#include <string>
#include <type_traits>
#include "char_convert.hpp"
namespace atithmetic_cvt {
	namespace detail {
		using char_cvt::u16tou8;
		using char_cvt::u32tou8;
		using char_cvt::u8tou16;
		using char_cvt::u8tou32;
		using std::nullptr_t;
		template<typename char_type, typename T>
		struct to_str_helper;
		template<typename T>
		struct to_str_helper<char, T> {
			std::string operator()(T n) { return std::to_string(n); }
		};
		template<typename T>
		struct to_str_helper<wchar_t, T> {
			std::wstring operator()(T n) { return std::to_wstring(n); }
		};
		template<typename T>
		struct to_str_helper<char16_t, T> {
			std::u16string operator()(T n) {
				if (sizeof(wchar_t) == 2) {
					return static_cast<std::u16string>(std::to_wstring(n));
				}
				else {
					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcvt;
					return u8tou16(wcvt.to_bytes(std::to_wstring(n)));
				}
			}
		};
		template<typename T>
		struct to_str_helper<char32_t, T> {
			std::u32string operator()(T n) {
				if (sizeof(wchar_t) == 2) {
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wcvt;
					return u8tou32(wcvt.to_bytes(std::to_wstring(n)));
				}
				else {
					return static_cast<std::u32string>(std::to_wstring(n));
				}
			}
		};
	}
	template<typename char_type, typename T, std::enable_if_t < std::is_arithmetic<T>::value, nullptr_t> = nullptr>
	inline std::basic_string<char_type> to_str(T n) {
		return detail::to_str_helper<char_type, T>()(n);
	}
	namespace detail {
		namespace stl_wrap {
			template<typename T> struct is_char_type_old : std::false_type {};
			template<> struct is_char_type_old<char> : std::true_type {};
			template<> struct is_char_type_old<wchar_t> : std::true_type {};
			template <typename char_type> int stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, int) {
				return std::stoi(s, idx, base);
			}
			template <typename char_type> long stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, long) {
				return std::stol(s, idx, base);
			}
			template <typename char_type> long long stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, long long) {
				return std::stoll(s, idx, base);
			}
			template <typename char_type> unsigned int stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, unsigned int) {
				const unsigned long re = std::stoul(s, idx, base);
				if (std::numeric_limits<unsigned int>::max() < re) throw std::out_of_range("in function atithmetic_cvt::detail::stl_wrap::stox_impl(int)");
				return static_cast<unsigned int>(re);
			}
			template <typename char_type> unsigned long stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, unsigned long) {
				return std::stoul(s, idx, base);
			}
			template <typename char_type> unsigned long long stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, unsigned long long) {
				return std::stoull(s, idx, base);
			}
			template<typename T, typename char_type, std::enable_if_t<is_char_type_old<char_type>::value && std::is_integral<T>::value, nullptr_t> = nullptr>
			T stox(const std::basic_string<char_type>& s, std::size_t * idx = nullptr, int base = 10) {
				return stox_impl(s, idx, base, T{});
			}
			template <typename char_type> float stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, float) {
				return std::stof(s, idx);
			}
			template <typename char_type> double stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, double) {
				return std::stod(s, idx);
			}
			template <typename char_type> long double stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, long double) {
				return std::stold(s, idx);
			}
			template<typename T, typename char_type, std::enable_if_t<is_char_type_old<char_type>::value && std::is_floating_point<T>::value, nullptr_t> = nullptr>
			inline T stox(const std::basic_string<char_type>& s, std::size_t * idx = nullptr) {
				return stox_impl(s, idx, T{});
			}
		}
		template<typename char_type, typename T>
		struct from_str_helper;
		template<typename T>
		struct from_str_helper<char, T> {
			T operator()(const std::string& s) {
				return stl_wrap::stox<T>(s);
			}
		};
		template<typename T>
		struct from_str_helper<wchar_t, T> {
			T operator()(const std::wstring& s) {
				return stl_wrap::stox<T>(s);
			}
		};
		template<typename T>
		struct from_str_helper<char16_t, T> {
			T operator()(const std::string& s) {
				if (sizeof(wchar_t) == 2) {
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wcvt;
					return stl_wrap::stox<T>(u16tou8(u8u16cvt.to_bytes(s)));
				}
				else {
					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcvt;
					return stl_wrap::stox<T>(u16tou8(u8u16cvt.to_bytes(s)));
				}
			}
		};
		template<typename T>
		struct from_str_helper<char32_t, T> {
			T operator()(const std::string& s) {
				if (sizeof(wchar_t) == 2) {
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wcvt;
					return stl_wrap::stox<T>(wcvt.from_bytes(u32tou8(s)));
				}
				else {
					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcvt;
					return stl_wrap::stox<T>(wcvt.from_bytes(u32tou8(s)));
				}
			}
		};
	}
	template<typename T, typename char_type, std::enable_if_t<std::is_arithmetic<T>::value, nullptr_t> = nullptr>
	inline T from_str(const std::basic_string<char_type>& s) {
		return detail::from_str_helper<char_type, T>()(s);
	}
}
#endif
