#pragma once

#include <dt/dt_interface.h>
#include <stdutils/io.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>


namespace delaunay
{

// Registration
template <typename F, typename I>
using ImplFactory = std::function<std::unique_ptr<Interface<F, I>>(const stdutils::io::ErrorHandler* err_handler)>;

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

// Convenience function to build the algorithm implementation with appropriate error handling
template <typename F, typename I = std::uint32_t>
std::unique_ptr<Interface<F, I>> make_dt_algo(const RegisteredImpl<F, I>& registered_impl, const stdutils::io::ErrorHandler* err_handler = nullptr);

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

template <typename F, typename I>
std::unique_ptr<Interface<F, I>> make_dt_algo(const RegisteredImpl<F, I>& registered_impl, const stdutils::io::ErrorHandler* err_handler)
{
    if (err_handler == nullptr)
    {
        return registered_impl.impl_factory(nullptr);
    }
    stdutils::io::ErrorHandler err_handler_with_algo_name = [err_handler_cpy = *err_handler, &registered_impl](stdutils::io::SeverityCode code, stdutils::io::ErrorMessage msg) {
        std::stringstream out;
        out << registered_impl.name << ": " << msg;
        err_handler_cpy(code, out.str());
    };
    return registered_impl.impl_factory(&err_handler_with_algo_name);
}

} // namespace delaunay
