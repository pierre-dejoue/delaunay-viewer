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

// Data structures
template <typename F, typename I>
using ImplFactory = std::function<std::unique_ptr<Interface<F, I>>(const stdutils::io::ErrorHandler* err_handler)>;

template <typename F, typename I>
struct RegisteredImpl
{
    std::string name;
    int confidence_score;
    ImplFactory<F, I> impl_factory;
};

template <typename F, typename I>
struct RegisteredImplList
{
    std::string reference;
    std::vector<RegisteredImpl<F, I>> algos;
};

// Registration
template <typename F, typename I>
bool register_impl(std::string_view name, const ImplFactory<F, I>& impl_factory);

// Should be called once
bool register_all_implementations();

// Query Delaunay implementations
template <typename F, typename I = std::uint32_t>
RegisteredImplList<F, I> get_impl_list();

// Convenience function to build the algorithm implementation with appropriate error handling
template <typename F, typename I = std::uint32_t>
std::unique_ptr<Interface<F, I>> make_dt_algo(const RegisteredImpl<F, I>& registered_impl, const stdutils::io::ErrorHandler* err_handler = nullptr);

// Same for the reference implementation
template <typename F, typename I = std::uint32_t>
std::pair<const std::string, std::unique_ptr<Interface<F, I>>> make_ref_dt_algo(const stdutils::io::ErrorHandler* err_handler = nullptr);

//
// Implementation
//
namespace details
{

    template <typename F, typename I>
    std::map<std::string, RegisteredImpl<F, I>>& get_impl_map()
    {
        static std::map<std::string, RegisteredImpl<F, I>> impl_map;
        return impl_map;
    }

    struct RefImpl
    {
        int score = -1;
        std::string name = "";
    };

    template <typename F, typename I>
    RefImpl& get_ref_impl()
    {
        static RefImpl ref_impl{};
        return ref_impl;
    }

} // namespace details

template <typename F, typename I>
bool register_impl(std::string_view name, int score, const ImplFactory<F, I>& impl_factory)
{
    auto& impl_map = details::get_impl_map<F, I>();
    if (std::any_of(std::cbegin(impl_map), std::cend(impl_map), [score](const auto& kvp){ return kvp.second.confidence_score == score; }))
    {
        assert(0);  // Don't reuse scores
        return false;
    }
    const auto [it, succeded] = impl_map.try_emplace(std::string(name), RegisteredImpl<F, I>{ std::string(name), score, impl_factory });
    if (succeded)
    {
        auto& ref_impl = details::get_ref_impl<F, I>();
        if (score > ref_impl.score)
        {
            ref_impl.score = score;
            ref_impl.name = name;
        }
    }
    return succeded;
}

template <typename F, typename I>
RegisteredImplList<F, I> get_impl_list()
{
    RegisteredImplList<F, I> result;
    const auto& impl_map = details::get_impl_map<F, I>();
    std::transform(std::cbegin(impl_map), std::cend(impl_map), std::back_inserter(result.algos), [](const auto& kvp) { return kvp.second; });
    result.reference = result.algos.empty() ? "" : details::get_ref_impl<F, I>().name;
    return result;
}

template <typename F, typename I>
std::unique_ptr<Interface<F, I>> make_dt_algo(const RegisteredImpl<F, I>& registered_impl, const stdutils::io::ErrorHandler* err_handler)
{
    if (err_handler == nullptr)
        return registered_impl.impl_factory(nullptr);

    // This handler is copied by the Interface<F, I> ctor
    stdutils::io::ErrorHandler err_handler_with_algo_name = [err_handler_cpy = *err_handler, &registered_impl](stdutils::io::SeverityCode code, stdutils::io::ErrorMessage msg) {
        std::stringstream out;
        out << registered_impl.name << ": " << msg;
        err_handler_cpy(code, out.str());
    };
    return registered_impl.impl_factory(&err_handler_with_algo_name);
}

template <typename F, typename I>
std::pair<const std::string, std::unique_ptr<Interface<F, I>>> make_ref_dt_algo(const stdutils::io::ErrorHandler* err_handler)
{
    auto ref_algo_name = details::get_ref_impl<F, I>().name;
    return std::make_pair<std::string, std::unique_ptr<Interface<F, I>>>(
        std::move(ref_algo_name),
        make_dt_algo(details::get_impl_map<F, I>().at(ref_algo_name), err_handler)
    );
}

} // namespace delaunay
