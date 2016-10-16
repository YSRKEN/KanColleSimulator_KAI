#ifndef KCS_KAI_INC_KAMMUSU_STD_FUTURE_HPP_
#define KCS_KAI_INC_KAMMUSU_STD_FUTURE_HPP_
#include <cstddef>
namespace std_future{
	template <class C> 
	constexpr auto size(const C& c) -> decltype(c.size()) { return c.size(); }
	template <class T, std::size_t N>
	constexpr std::size_t size(const T (&array)[N]) noexcept { return N; }
}
#endif //KCS_KAI_INC_KAMMUSU_STD_FUTURE_HPP_