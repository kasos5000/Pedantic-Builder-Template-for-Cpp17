#pragma once

#include <tuple>
#include <array>

using std::size_t;

namespace PBT_Inner {
template<bool... Marks>
class SettingMarkSequence {
public:
	static constexpr size_t SequenceSize = sizeof...(Marks);

	template<size_t I = 0>
	static constexpr size_t SettingProgress() {
		if constexpr (!sequence_itself[I]) {
			return I;
		} else if constexpr (I < SequenceSize - 1) {
			return SettingProgress<I + 1>();
		} else {
			return SequenceSize;
		}
	}

	template<template<bool...> typename UpdatedSequence, size_t NumOfArg, size_t... Idx>
	static constexpr auto GenerateBoolSequence(std::index_sequence<Idx...>) {
		static_assert(!sequence_itself[NumOfArg], "This arg has already been set");
		return UpdatedSequence<CurMark<Idx, NumOfArg>...>();
	}

	template<size_t NumOfArg>
	using UpdatedSequence = decltype(GenerateBoolSequence<SettingMarkSequence, NumOfArg>(std::make_index_sequence<SequenceSize>{}));

private:
	static constexpr std::array<bool, SequenceSize> sequence_itself = {Marks...};
	template<size_t I, size_t NumOfArg>
	static constexpr bool CurMark = (I == NumOfArg) ? true : sequence_itself[I];
};

template<size_t... MoveList>
class MoveSelector {
public:
	template<size_t Index, size_t Iter = 0>
	static constexpr bool ListHasIndex() {
		if constexpr (!ListSize) {
			return true;
		} else if constexpr (move_list[Iter] == Index) {
			return true;
		} else if constexpr (Iter < ListSize - 1) {
			return ListHasIndex<Index, Iter + 1>();
		} else {
			return false;
		}
	}

	static constexpr bool IsEmpty() {
		return !ListSize;
	}

private:
	static constexpr size_t ListSize = sizeof...(MoveList);
	static constexpr std::array<size_t, ListSize> move_list{MoveList...};
};

template<typename FullClassName>
struct TemplateNameStripper;

template<template<bool...> typename TemplateName, bool... Coefs>
struct TemplateNameStripper<TemplateName<Coefs...>*> {
	template<bool... Bs> using BuilderTemplateName = TemplateName<Bs...>;
};

template<template<class...> class CoreTempName, class... ArgTs>
struct TemplateNameStripper<CoreTempName<ArgTs...>> {
	static constexpr size_t PackSize = sizeof...(ArgTs);
	template<size_t I> using ArgType = std::tuple_element_t<I, std::tuple<ArgTs...>>;
};
} // namespace PBT_Inner

#define PBT_SET_ARG_INNER(Number, value) this->template SetArg<PBT_Inner::TemplateNameStripper<decltype(this)>::template BuilderTemplateName, Number>(value)

namespace pbt {
struct none {};
struct move_or_derefer {};
using move = move_or_derefer;
using derefer = move_or_derefer;
} // namespace pbt

