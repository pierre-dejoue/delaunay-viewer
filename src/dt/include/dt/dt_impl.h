#pragma once

#include <dt/dt_interface.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace delaunay
{

// Registration
template <typename F, typename I>
using ImplFactory = std::function<std::unique_ptr<Interface<F, I>>(void)>;

template <typename F, typename I>
bool register_impl(std::string_view name, const ImplFactory<F, I>& impl_factory);

// Should be called once
bool register_all_implementations();

// Access delaunay implementations
template <typename F, typename I>
struct RegisteredImpl
{
    std::string name;
    ImplFactory<F, I> impl_factory;
};
template <typename F, typename I = std::uint32_t>
std::vector<RegisteredImpl<F, I>> get_impl_list();

//
// Implementation
//
template <typename F, typename I>
std::map<std::string, ImplFactory<F, I>>& get_impl_map()
{
    static std::map<std::string, ImplFactory<F, I>> impl_map;
    return impl_map;
}

template <typename F, typename I>
bool register_impl(std::string_view name, const ImplFactory<F, I>& impl_factory)
{
    auto& impl_map = get_impl_map<F, I>();
    const auto [it, succeded] = impl_map.try_emplace(std::string(name), impl_factory);
    return succeded;
}

template <typename F, typename I>
std::vector<RegisteredImpl<F, I>> get_impl_list()
{
    std::vector<RegisteredImpl<F, I>> result;
    const auto& impl_map = get_impl_map<F, I>();
    std::transform(std::cbegin(impl_map), std::cend(impl_map), std::back_inserter(result), [](const auto& kvp) { return RegisteredImpl<F, I>{ kvp.first, kvp.second }; });
    return result;
}

} // namespace delaunay
