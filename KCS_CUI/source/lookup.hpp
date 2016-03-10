#pragma once
#include <cstddef>		// std::size_t
#include <stdexcept>	// std::invalid_argument
#include <type_traits>	// std::add_const_t
#include <utility>		// std::get, std::tuple_element_t

/// <summary>ペア配列の中からキーが含まれているかを検索します。</summary>
/// <param name="pairs">ペア配列。std::pair配列、std::tuple配列、std::array配列のいずれかが使えます。</param>
/// <param name="key">検索するキー。std::pairの場合はfirst、std::tupleやstd::arrayの場合は最初の要素と比較します。</param>
/// <param name="i">検索を開始するインデックス。省略した場合は先頭から検索します。</param>
/// <returns>キーが含まれている場合はtrue。</returns>
/// <remarks>型推論によりテンプレート引数は全て省略可能です。</remarks>
template<class T, std::size_t N, class Key = std::add_const_t<std::tuple_element_t<0, T>>>
constexpr bool Contains(const T (&pairs)[N], Key& key, std::size_t i = 0) {
	return i != N && (std::get<0>(pairs[i]) == key || Contains(pairs, key, i + 1));
}

/// <summary>ペア配列の中からキーを検索し最初に見つけた値を返します。見つからなかった場合は指定されたデフォルト値を返します。</summary>
/// <param name="pairs">ペア配列。std::pair配列、std::tuple配列、std::array配列のいずれかが使えます。</param>
/// <param name="key">検索するキー。std::pairの場合はfirst、std::tupleやstd::arrayの場合は最初の要素と比較します。</param>
/// <param name="defaultValue">見つからなかった場合に返すデフォルト値。</param>
/// <param name="i">検索を開始するインデックス。省略した場合は先頭から検索します。</param>
/// <returns>最初に見つけた値。std::pairの場合はsecond、std::tupleやstd::arrayの場合は２番目の要素になります。見つからなかった場合は指定されたデフォルト値。</returns>
/// <remarks>型推論によりテンプレート引数は全て省略可能です。</remarks>
template<class T, std::size_t N, class Key = std::add_const_t<std::tuple_element_t<0, T>>, class Value = std::tuple_element_t<1, T>>
constexpr Value FirstOrDefault(const T (&pairs)[N], Key& key, Value defaultValue, std::size_t i = 0) {
	return i == N ? defaultValue
		: std::get<0>(pairs[i]) == key ? std::get<1>(pairs[i])
		: FirstOrDefault(pairs, key, defaultValue, i + 1);
}

/// <summary>ペア配列の中からキーを検索し見つけた値を返します。見つからなかった場合や複数見つかった場合は例外を投げます。</summary>
/// <param name="pairs">ペア配列。std::pair配列、std::tuple配列、std::array配列のいずれかが使えます。</param>
/// <param name="key">検索するキー。std::pairの場合はfirst、std::tupleやstd::arrayの場合は最初の要素と比較します。</param>
/// <param name="i">検索を開始するインデックス。省略した場合は先頭から検索します。</param>
/// <returns>見つけた値。std::pairの場合はsecond、std::tupleやstd::arrayの場合は２番目の要素になります。</returns>
/// <exception cref="std::invalid_argument">見つからなかった場合や複数見つかった場合に投げられます。</exception>
/// <remarks>型推論によりテンプレート引数は全て省略可能です。</remarks>
template<class T, std::size_t N, class Key = std::add_const_t<std::tuple_element_t<0, T>>, class Value = std::tuple_element_t<1, T>>
constexpr Value Single(const T (&pairs)[N], Key& key, std::size_t i = 0) {
	return i == N ? throw std::invalid_argument("not found.")
		: std::get<0>(pairs[i]) != key ? Single(pairs, key, i + 1)
		: Contains(pairs, key, i + 1) ? throw std::invalid_argument("duplicated key.")
		: std::get<1>(pairs[i]);
}
