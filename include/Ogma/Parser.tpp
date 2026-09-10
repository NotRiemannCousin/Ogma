#pragma once

//! @file Ogma/Parser.tpp
//! @brief Definitions of the public Ogma parser and execution API.
//!
//! Everything involving the default (struct-walking) TypeParser engine —
//! member setters, the flag/positional tables, the token loop — lives in
//! @c <Ogma/TypeParser/_base.tpp>. This file only binds the public entry
//! points onto that engine.

#include <cstdlib>
#include <expected>
#include <format>
#include <meta>
#include <print>
#include <variant>

namespace Ogma {

    template<class T>
    Result<T> Parse(ArgSpan args) {
        auto parsed{ TypeParser<T, details_::internalArg>::Parse(T{}, "", args) };
        if (!parsed)
            return std::unexpected{ std::move(parsed).error() };

        if (!parsed->second.empty())
            return std::unexpected{ std::format("unexpected argument: '{}'", parsed->second.front()) };

        return std::move(parsed);
    }

    template<class T>
    std::expected<int, std::string> Execute(const T& config) {
        if constexpr (requires { config.operator()(); }) {
            return config();
        } else if constexpr (Utils::IsSpecializationOf<T>(^^std::variant)) {
            return std::visit([](const auto& alternative) -> std::expected<int, std::string> {
                return Execute(alternative);
            }, config);
        } else {
            std::expected<int, std::string> result{ 0 };
            bool handled{};

            static constexpr auto members{ std::define_static_array(
                std::meta::nonstatic_data_members_of(
                    ^^T, std::meta::access_context::current())) };

            template for (constexpr auto member : members) {
                if (!handled) {
                    using MemberT = TypeOf<member>;
                    if constexpr (Utils::IsSpecializationOf<MemberT>(^^std::variant)) {
                        result = std::visit(
                            [](const auto& alternative) -> std::expected<int, std::string> {
                                return Execute(alternative);
                            }, config.[: member :]);
                        handled = true;
                    }
                }
            }

            return result;
        }
    }

    template<class T>
    int ParseAndExecute(ArgSpan args) {
        const auto parsed{ [&]() -> Result<T> {
            try {
                return Parse<T>(args);
            } catch (const HelpRequested& help) {
                std::print("{}", help.text);
                return std::unexpected{ std::string{} };
            }
        }() };

        if (!parsed) {
            if (!parsed.error().empty())
                std::println(stderr, "error: {}", parsed.error());
            return parsed.error().empty() ? EXIT_SUCCESS : EXIT_FAILURE;
        }

        auto executed{ Execute(parsed->first) };
        if (!executed) {
            if (!executed.error().empty())
                std::println(stderr, "error: {}", executed.error());
            return EXIT_FAILURE;
        }

        return *executed;
    }

}