#pragma once
#include <stdexcept>	// std::invalid_argument
#include <type_traits>	// std::add_const_t, std::add_lvalue_reference_t
#include <utility>		// std::get, std::tuple_element_t

namespace detail {
	const int block_size = 128;

	template<class T, int N, class Key>
	constexpr int IndexOfBlock(const T (&pairs)[N], const Key& key, int offset, int i = 0) noexcept {
		return i == block_size || N <= offset + i ? -1
			: std::get<0>(pairs[offset + i]) == key ? offset + i
			: IndexOfBlock(pairs, key, offset, i + 1);
	}
}

/// <summary>ペア配列の中からキーが含まれているかを検索します。</summary>
/// <param name="pairs">ペア配列。std::pair配列、std::tuple配列、std::array配列のいずれかが使えます。</param>
/// <param name="key">検索するキー。std::pairの場合はfirst、std::tupleやstd::arrayの場合は最初の要素と比較します。</param>
/// <param name="offset">検索を開始するインデックス。省略した場合は先頭から検索します。</param>
/// <returns>キーが含まれている場合はtrue。</returns>
/// <remarks>型推論によりテンプレート引数は全て省略可能です。</remarks>
template<class T, int N, class Key = std::add_lvalue_reference_t<std::add_const_t<std::tuple_element_t<0, T>>>>
constexpr bool Contains(const T (&pairs)[N], Key key, int offset = 0) noexcept {
	return offset < N && (0 <= detail::IndexOfBlock(pairs, key, offset) || Contains(pairs, key, offset + detail::block_size));
}

/// <summary>ペア配列の中からキーを検索し最初に見つけた値を返します。見つからなかった場合は例外を投げます。</summary>
/// <param name="pairs">ペア配列。std::pair配列、std::tuple配列、std::array配列のいずれかが使えます。</param>
/// <param name="key">検索するキー。std::pairの場合はfirst、std::tupleやstd::arrayの場合は最初の要素と比較します。</param>
/// <param name="offset">検索を開始するインデックス。省略した場合は先頭から検索します。</param>
/// <returns>最初に見つけた値。std::pairの場合はsecond、std::tupleやstd::arrayの場合は２番目の要素になります。</returns>
/// <exception cref="std::invalid_argument">見つからなかった場合に投げられます。</exception>
/// <remarks>型推論によりテンプレート引数は全て省略可能です。</remarks>
template<class T, int N, class Key = std::add_lvalue_reference_t<std::add_const_t<std::tuple_element_t<0, T>>>, class Value = std::tuple_element_t<1, T>>
constexpr Value First(const T (&pairs)[N], Key key, int offset = 0) {
	return N <= offset ? throw std::invalid_argument("not found.")
		: 0 <= detail::IndexOfBlock(pairs, key, offset) ? std::get<1>(pairs[detail::IndexOfBlock(pairs, key, offset)])
		: First(pairs, key, offset + detail::block_size);
}

/// <summary>ペア配列の中からキーを検索し最初に見つけた値を返します。見つからなかった場合は指定されたデフォルト値を返します。</summary>
/// <param name="pairs">ペア配列。std::pair配列、std::tuple配列、std::array配列のいずれかが使えます。</param>
/// <param name="key">検索するキー。std::pairの場合はfirst、std::tupleやstd::arrayの場合は最初の要素と比較します。</param>
/// <param name="defaultValue">見つからなかった場合に返すデフォルト値。</param>
/// <param name="offset">検索を開始するインデックス。省略した場合は先頭から検索します。</param>
/// <returns>最初に見つけた値。std::pairの場合はsecond、std::tupleやstd::arrayの場合は２番目の要素になります。見つからなかった場合は指定されたデフォルト値。</returns>
/// <remarks>型推論によりテンプレート引数は全て省略可能です。</remarks>
template<class T, int N, class Key = std::add_lvalue_reference_t<std::add_const_t<std::tuple_element_t<0, T>>>, class Value = std::tuple_element_t<1, T>>
constexpr Value FirstOrDefault(const T (&pairs)[N], Key key, Value defaultValue, int offset = 0) noexcept {
	return N <= offset ? defaultValue
		: 0 <= detail::IndexOfBlock(pairs, key, offset) ? std::get<1>(pairs[detail::IndexOfBlock(pairs, key, offset)])
		: FirstOrDefault(pairs, key, defaultValue, offset + detail::block_size);
}

/// <summary>ペア配列の中からキーを検索し見つけた値を返します。見つからなかった場合や複数見つかった場合は例外を投げます。</summary>
/// <param name="pairs">ペア配列。std::pair配列、std::tuple配列、std::array配列のいずれかが使えます。</param>
/// <param name="key">検索するキー。std::pairの場合はfirst、std::tupleやstd::arrayの場合は最初の要素と比較します。</param>
/// <param name="offset">検索を開始するインデックス。省略した場合は先頭から検索します。</param>
/// <returns>見つけた値。std::pairの場合はsecond、std::tupleやstd::arrayの場合は２番目の要素になります。</returns>
/// <exception cref="std::invalid_argument">見つからなかった場合や複数見つかった場合に投げられます。</exception>
/// <remarks>型推論によりテンプレート引数は全て省略可能です。</remarks>
template<class T, int N, class Key = std::add_lvalue_reference_t<std::add_const_t<std::tuple_element_t<0, T>>>, class Value = std::tuple_element_t<1, T>>
constexpr Value Single(const T (&pairs)[N], Key key, int offset = 0) {
	return N <= offset ? throw std::invalid_argument("not found.")
		: detail::IndexOfBlock(pairs, key, offset) < 0 ? Single(pairs, key, offset + detail::block_size)
		: Contains(pairs, key, detail::IndexOfBlock(pairs, key, offset) + 1) ? throw std::invalid_argument("duplicated key.")
		: std::get<1>(pairs[detail::IndexOfBlock(pairs, key, offset)]);
}
