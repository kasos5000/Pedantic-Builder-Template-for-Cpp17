#pragma once

#include "builder_common.hpp"

namespace PBT_Inner {
template<typename Core, typename Shell>
class PrivateBuilderT;

template<typename Core, bool... Marks>
class PrivateBuilderT<Core, SettingMarkSequence<Marks...>> : public Core {
private:
	static constexpr size_t SettingProgress = SettingMarkSequence<Marks...>::SettingProgress();
	template<size_t I> using _ArgType = typename PrivateBuilderT::template ArgType<I>;
	template<typename Core2, typename Shell2> friend class PrivateBuilderT;

public:
	using Core::Core;

	template<typename TargetType, class Option = pbt::none, size_t... MoveIdx>
	auto Construct() {
		return ConstructInner<TargetType, Option, PBT_Inner::MoveSelector<MoveIdx...>>(std::make_index_sequence<SettingProgress>());
	}
	template<typename TargetType, class Option = pbt::none, size_t... MoveIdx>
	auto Construct() const {
		return ConstructInner<TargetType, Option, PBT_Inner::MoveSelector<MoveIdx...>>(std::make_index_sequence<SettingProgress>());
	}

	template<typename TargetType, class Option = pbt::none, size_t... MoveIdx>
	auto ConstructNew() {
		return ConstructNewInner<TargetType, Option, PBT_Inner::MoveSelector<MoveIdx...>>(std::make_index_sequence<SettingProgress>());
	}
	template<typename TargetType, class Option = pbt::none, size_t... MoveIdx>
	auto ConstructNew() const {
		return ConstructNewInner<TargetType, Option, PBT_Inner::MoveSelector<MoveIdx...>>(std::make_index_sequence<SettingProgress>());
	}

	template<class Option = pbt::none, size_t... MoveIdx, typename Func>
	decltype(auto) Invoke(Func&& func) {
		return InvokeInner<Option, PBT_Inner::MoveSelector<MoveIdx...>>(func, std::make_index_sequence<SettingProgress>());
	}
	template<class Option = pbt::none, size_t... MoveIdx, typename Func>
	decltype(auto) Invoke(Func&& func) const {
		return InvokeInner<Option, PBT_Inner::MoveSelector<MoveIdx...>>(func, std::make_index_sequence<SettingProgress>());
	}

protected:
	~PrivateBuilderT() = default;

	template<template<bool...> typename FinalBuilder, size_t I, typename Type>
	auto SetArg(Type&& value) {
		this->template GetArg<I>() = std::forward<Type>(value);
		using _UpdatedSequence = typename SettingMarkSequence<Marks...>::template UpdatedSequence<I>;
		return PrivateBuilderT<Core, _UpdatedSequence>::template MakeFinalBuilder<FinalBuilder>(this);
	}

private:
	template<template<bool...> typename FinalBuilder>
	static auto MakeFinalBuilder(Core* ptr) {
		return FinalBuilder<Marks...>(ptr);
	}

	template<size_t I, typename Selector_>
	auto&& MoveIfSelected() {
		if constexpr (Selector_::IsEmpty()) {
			return std::move(this->template GetArg<I>());
		} else if constexpr (Selector_::template ListHasIndex<I>()) {
			if constexpr (std::is_pointer_v<_ArgType<I>>) {
				*this->template GetArg<I>();
			} else {
				return std::move(this->template GetArg<I>());
			}
		} else {
			return this->template GetArg<I>();
		}
	}

	template<typename TargetType, class Option, class Selector_, size_t... Idx>
	auto ConstructInner(std::index_sequence<Idx...>) {
		if constexpr (std::is_same_v<Option, pbt::none>) {
			return TargetType(this->template GetArg<Idx>()...);
		} else {
			return TargetType(MoveIfSelected<Idx, Selector_>()...);
		}
	}
	template<typename TargetType, class Option, class Selector_, size_t... Idx>
	auto ConstructInner(std::index_sequence<Idx...>) const {
		return TargetType(this->template GetArg<Idx>()...);
	}

	template<typename TargetType, class Option, class Selector_, size_t... Idx>
	auto ConstructNewInner(std::index_sequence<Idx...>) {
		if constexpr (std::is_same_v<Option, pbt::none>) {
			return new TargetType(this->template GetArg<Idx>()...);
		} else {
			return new TargetType(MoveIfSelected<Idx, Selector_>()...);
		}
	}
	template<typename TargetType, class Option, class Selector_, size_t... Idx>
	auto ConstructNewInner(std::index_sequence<Idx...>) const {
		return new TargetType(this->template GetArg<Idx>()...);
	}

	template<class Option, class Selector_, typename Func, size_t... Idx>
	decltype(auto) InvokeInner(Func&& func, std::index_sequence<Idx...>) {
		if constexpr (std::is_same_v<Option, pbt::none>) {
			return func(this->template GetArg<Idx>()...);
		} else {
			return func(MoveIfSelected<Idx, Selector_>()...);
		}
	}
	template<class Option, class Selector_, typename Func, size_t... Idx>
	decltype(auto) InvokeInner(Func&& func, std::index_sequence<Idx...>) const {
		return func(this->template GetArg<Idx>()...);
	}
};
} // namespace PBT_Inner

template<typename Core, bool... Marks>
class PrivateBuilder : public PBT_Inner::PrivateBuilderT<Core, PBT_Inner::SettingMarkSequence<Marks...>> {
public:
	using PBT_Inner::PrivateBuilderT<Core, PBT_Inner::SettingMarkSequence<Marks...>>::PrivateBuilderT;

protected:
	~PrivateBuilder() = default;
};