#pragma once

#include "pbt_inner/shared_builder_t.hpp"
#include "pbt_inner/ghost_tuple.hpp"

template<typename... Ts>
class SimpleSharedCore {
public:
	template<size_t I>
	static constexpr auto&& GetArg() {
		return std::get<I>(data);
	}

	static void Reset() {
		data = std::tuple<Ts...>();
	}

protected:
	~SimpleSharedCore() = default;

private:
	inline static std::tuple<Ts...> data;
};

template<typename... Ts>
class FullySharedCore {
public:
	template<size_t I>
	static constexpr auto&& GetArg() {
		if constexpr (std::is_reference_v<ArgType<I>>) {
			return std::forward<ArgType<I>>(Data::template Get<I>());
		} else {
			return Data::template Get<I>();
		}
	}

protected:
	~FullySharedCore() = default;
	template<size_t I> using ArgType = std::tuple_element_t<I, GhostTuple<Ts...>>;

private:
	using Data = GhostTuple<Ts...>;
};
