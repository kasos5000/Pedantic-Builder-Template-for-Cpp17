#pragma once

#include "builder_common.hpp"

namespace PBT_Inner {
template<typename Core, bool... Marks>
class SharedBuilder : public Core {
private:
	static constexpr size_t SettingProgress = SettingMarkSequence<Marks...>::SettingProgress();
	template<size_t I> using _ArgType = typename SharedBuilder::template ArgType<I>;

public:
	template<typename ReturnType, typename Option = pbt::none, size_t... MoveIdx>
	auto Construct() const {
		return ConstructInner<ReturnType, Option, MoveSelector<MoveIdx...>>(std::make_index_sequence<SettingProgress>());
	}

	template<typename ReturnType, typename Option = pbt::none, size_t... MoveIdx>
	auto ConstructNew() const {
		return ConstructNewInner<ReturnType, Option, MoveSelector<MoveIdx...>>(std::make_index_sequence<SettingProgress>());
	}

	template<typename Option = pbt::none, size_t... MoveIdx, typename Func>
	decltype(auto) Invoke(Func&& func) const {
		return InvokeInner<Option, MoveSelector<MoveIdx...>>(func, std::make_index_sequence<SettingProgress>());
	}

protected:
	~SharedBuilder() = default;

	template<template<bool...> typename FinalBuilder, size_t I, typename Type>
	static auto SetArg(Type&& value) {
		SharedBuilder::template GetArg<I>() = std::forward<Type>(value);
		return SettingMarkSequence<Marks...>::template GenerateBoolSequence<FinalBuilder, I>(std::make_index_sequence<sizeof...(Marks)>());
	}

private:
	template<size_t I, typename Selector_>
	static constexpr auto&& MoveIfSelected() {
		if constexpr (Selector_::template ListHasIndex<I>()) {
			if constexpr (std::is_pointer_v<_ArgType<I>> && !Selector_::IsEmpty()) {
				return *SharedBuilder::template GetArg<I>();
			} else {
				return std::move(SharedBuilder::template GetArg<I>());
			}
		} else {
			return SharedBuilder::template GetArg<I>();
		}
	}

	template<typename ReturnType, typename Option, typename Selector_, size_t... Idx>
	static auto ConstructInner(std::index_sequence<Idx...>) {
		if constexpr (std::is_same_v<Option, pbt::move_or_derefer>) {
			return ReturnType(MoveIfSelected<Idx, Selector_>()...);
		} else {
			return ReturnType(SharedBuilder::template GetArg<Idx>()...);
		}
	}

	template<typename ReturnType, typename Option, typename Selector_, size_t... Idx>
	static auto ConstructNewInner(std::index_sequence<Idx...>) {
		if constexpr (std::is_same_v<Option, pbt::move_or_derefer>) {
			return new ReturnType(MoveIfSelected<Idx, Selector_>()...);
		} else {
			return new ReturnType(SharedBuilder::template GetArg<Idx>()...);
		}
	}

	template<typename Option, typename Selector_, typename Func, size_t... Idx>
	static decltype(auto) InvokeInner(Func&& func, std::index_sequence<Idx...>) {
		if constexpr (std::is_same_v<Option, pbt::move_or_derefer>) {
			return func(MoveIfSelected<Idx, Selector_>()...);
		} else {
			return func(SharedBuilder::template GetArg<Idx>()...);
		}
	}
};
} // namespace PBT_Inner

template<typename Core, bool... Marks>
using SharedBuilder = PBT_Inner::SharedBuilder<Core, Marks...>;
