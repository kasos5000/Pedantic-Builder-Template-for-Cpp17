#pragma once

#include <utility>

template<typename... Tail>
class GhostTuple;

template<typename Head, typename... Tail>
class GhostTuple<Head, Tail...> {
public:
	static constexpr std::size_t Size = sizeof...(Tail) + 1;

	template<std::size_t I>
	static constexpr auto&& Get() {
		if constexpr (I) {
			return GhostTuple<Tail...>::template Get<I - 1>();
		} else {
			return head;
		}
	}

	static void Set(Head&& head_, Tail&&... tail_) {
		head = std::forward<Head>(head_);
		GhostTuple<Tail...>::Set(std::forward<Tail>(tail_)...);
	}

private:
	inline static Head head = std::decay_t<Head>{};
};

template<>
class GhostTuple<> {
	static constexpr std::size_t Size = 0;
};

namespace std {
template<std::size_t I, class T>
struct tuple_element;

template<std::size_t I, class Head, class... Tail>
struct tuple_element<I, GhostTuple<Head, Tail...>> : std::tuple_element<I - 1, GhostTuple<Tail...>> {};

template<class Head, class... Tail>
struct tuple_element<0, GhostTuple<Head, Tail...>> {
	using type = Head;
};

template<class T>
struct tuple_size;

template<class... Types>
struct tuple_size<GhostTuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template<std::size_t I, class... Types>
typename tuple_element<I, GhostTuple<Types...>>::type& get(GhostTuple<Types...>&) noexcept {
	return GhostTuple<Types...>::template Get<I>();
}
template<std::size_t I, class... Types>
typename tuple_element<I, GhostTuple<Types...>>::type&& get(GhostTuple<Types...>&&) noexcept {
	return GhostTuple<Types...>::template Get<I>();
}
template<std::size_t I, class... Types>
const typename tuple_element<I, GhostTuple<Types...>>::type& get(const GhostTuple<Types...>&) noexcept {
	return GhostTuple<Types...>::template Get<I>();
}
template<std::size_t I, class... Types>
const typename tuple_element<I, GhostTuple<Types...>>::type&& get(const GhostTuple<Types...>&&) noexcept {
	return GhostTuple<Types...>::template Get<I>();
}
}
