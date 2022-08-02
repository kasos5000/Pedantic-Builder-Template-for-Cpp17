#pragma once

#include "pbt_inner/private_builder_t.hpp"
#include <memory>

template<typename... Ts>
class SimplePrivateCore {
public:
	SimplePrivateCore() :
	    data(std::make_shared<std::tuple<Ts...>>()) {
	}

	SimplePrivateCore(SimplePrivateCore* other) :
	    data(other->data) {
	}

	SimplePrivateCore(Ts&&... def_args) :
	    data(std::make_shared(std::tuple<Ts...>{std::forward<Ts>(def_args)...})) { /*1*/
	}

	template<size_t I>
	inline auto&& GetArg() {
		return std::get<I>(*data);
	}
	template<size_t I>
	inline const auto& GetArg() const {
		return std::get<I>(*data);
	}

protected:
	~SimplePrivateCore() = default;
	template<size_t I> using ArgType = std::tuple_element_t<I, std::tuple<Ts...>>;

private:
	std::shared_ptr<std::tuple<Ts...>> data;
};

/*1 If you'd like to use a core with rvalue-references you have to use this constructur*/

