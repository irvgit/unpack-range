#pragma once

#if __cplusplus < 202302L
    #error out of date c++ version, compile with -stdc++=2c
#endif

#include <cstdint>
#include <functional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace unpack_range {
    inline namespace range_unpacking {
        namespace detail {
            template <bool tp_as_iterators, std::uintmax_t tp_count>
            struct as_elements_or_iterators_fn : std::ranges::range_adaptor_closure<as_elements_or_iterators_fn<tp_as_iterators, tp_count>> {
                template <std::ranges::input_range tp_input_range_t>
                requires (std::move_constructible<std::ranges::range_reference_t<tp_input_range_t>>) //not needed if switched to aggregate tuple, as copy elision is garuenteed
                auto constexpr operator()(tp_input_range_t&& p_range) const -> auto {
                    return []<std::uintmax_t tp_index>(this auto p_self, auto p_current_iterator, auto... p_accumilated_iterators) {
                        if constexpr(tp_index < tp_count) {
                            auto l_current_element_iterator = p_current_iterator;
                            return p_self.template operator()<tp_index + 1>(
                                std::move(++p_current_iterator),
                                std::move(p_accumilated_iterators)...,
                                std::move(l_current_element_iterator)
                            );
                        }
                        else if constexpr (tp_as_iterators)
                            return std::tuple{p_accumilated_iterators...};
                        else return std::tuple<std::iter_reference_t<decltype(p_accumilated_iterators)>...>{*p_accumilated_iterators...};
                    }.template operator()<0>(std::ranges::begin(p_range));
                }
            };

            template <bool tp_as_iterators, std::uintmax_t tp_count>
            struct as_elements_or_iterators_with_rest_fn : std::ranges::range_adaptor_closure<as_elements_or_iterators_with_rest_fn<tp_as_iterators, tp_count>> {
                template <std::ranges::input_range tp_input_range_t>
                requires (std::move_constructible<std::ranges::range_reference_t<tp_input_range_t>>) //not needed if switched to aggregate tuple, as copy elision is garuenteed
                auto constexpr operator()(tp_input_range_t&& p_range) const -> auto {
                    return []<std::uintmax_t tp_index>(this auto p_self, auto p_end_iterator, auto p_current_iterator, auto... p_accumilated_iterators) {
                        if constexpr(tp_index < tp_count) {
                            auto l_current_element_iterator = p_current_iterator;
                            return p_self.template operator()<tp_index + 1>(
                                std::move(p_end_iterator),
                                std::move(++p_current_iterator),
                                std::move(p_accumilated_iterators)...,
                                std::move(l_current_element_iterator)
                            );
                        }
                        else if constexpr (tp_as_iterators)
                            return std::tuple{p_accumilated_iterators..., std::ranges::subrange{std::move(p_current_iterator), std::move(p_end_iterator)}};
                        else return std::tuple<std::iter_reference_t<decltype(p_accumilated_iterators)>..., decltype(std::ranges::subrange{std::move(p_current_iterator), std::move(p_end_iterator)})>{
                            *p_accumilated_iterators..., std::ranges::subrange{std::move(p_current_iterator), std::move(p_end_iterator)}
                        };
                    }.template operator()<0>(std::ranges::end(p_range), std::ranges::begin(p_range));
                }
            };
        }
        template <std::uintmax_t tp_count>
        auto constexpr as_elements = detail::as_elements_or_iterators_fn<false, tp_count>{};

        template <std::uintmax_t tp_count>
        auto constexpr as_elements_with_rest = detail::as_elements_or_iterators_with_rest_fn<false, tp_count>{};

        template <std::uintmax_t tp_count>
        auto constexpr as_iterators = detail::as_elements_or_iterators_fn<true, tp_count>{};

        template <std::uintmax_t tp_count>
        auto constexpr as_iterators_with_rest = detail::as_elements_or_iterators_with_rest_fn<true, tp_count>{};
    }
}
