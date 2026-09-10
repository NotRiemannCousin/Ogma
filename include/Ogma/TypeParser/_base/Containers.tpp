#pragma once
#include <print>

namespace Ogma {
    namespace details_ {
        template<auto InserterFn, auto... Annots, std::ranges::range T>
        auto ReadElements(T container, ArgSpan args) -> Result<T> {
            using Elem = std::ranges::range_value_t<T>;


            static constexpr auto propagatedAnnots{ std::define_static_array(Utils::GetForwardedAnnots<Annots...>()) };
            static constexpr auto isNamedOnly{ IsNamedOnly<Elem>(propagatedAnnots) };

            static constexpr auto parseFn{ std::meta::substitute(
                                                ^^TypeParser<Elem, internalArg>::Parse,
                                                propagatedAnnots) };

            constexpr auto isClosingToken{ [](const std::string_view token) {
                return Utils::IsFlag(token) || token == closingToken;
            } };

            while (!args.empty() && !isClosingToken(args[0])) {
                if constexpr (isNamedOnly) {
                    if (!args.empty() && args[0] == controlToken) args = args.subspan(1);
                    else return std::unexpected{ "\"--\" is required to open a container of named-only values" };
                }

                auto elem{ std::invoke([: parseFn :], Elem{}, "", args, std::string_view{}) };
                if (!elem) return std::unexpected{ std::move(elem).error() };

                std::invoke(InserterFn, container, std::move(elem->first));
                args = elem->second;
            }

            if (!args.empty() && args[0] == closingToken) args = args.subspan(1);
            return std::pair{ std::move(container), args };
        }


        template<auto InserterFn, class T, auto... Annots>
        Result<T> ReadContainer(T container, ArgSpan args) {
            namespace vs = std::views;
            namespace rg = std::ranges;


            if constexpr (Utils::GetAnnotation<Overwrite, Annots...>())
                container.clear();

            return ReadElements<InserterFn, Annots...>(std::move(container), args);
        }
    }


    template<class T, Arg ArgDefinition>
    template<auto... Annots>
    auto TypeParser<std::vector<T>, ArgDefinition>::Parse(
        Vector curr, std::string_view, const ArgSpan args, std::string_view) -> VectorRes {
        constexpr auto inserterFn{ [](Vector& container, T elem) {
            container.push_back(std::move(elem));
        } };

        return details_::ReadContainer<inserterFn, Vector, Annots...>(std::move(curr), args);
    }

    template<class T, class Compare, Arg ArgDefinition>
    template<auto... Annots>
    auto TypeParser<std::set<T, Compare>, ArgDefinition>::Parse(
        Set curr, std::string_view, const ArgSpan args, std::string_view) -> SetRes {
        constexpr auto inserterFn{ [](Set& container, T elem) {
            container.insert(std::move(elem));
        } };

        return details_::ReadContainer<inserterFn, Set, Annots...>(std::move(curr), args);
    }

    template<class T, class Hash, class Eq, Arg ArgDefinition>
    template<auto... Annots>
    auto TypeParser<std::unordered_set<T, Hash, Eq>, ArgDefinition>::Parse(
        UnorderedSet curr, std::string_view, const ArgSpan args, std::string_view) -> UnorderedSetRes {
        constexpr auto inserterFn{ [](UnorderedSet& container, T elem) {
            container.insert(std::move(elem));
        } };

        return details_::ReadContainer<inserterFn, UnorderedSet, Annots...>(std::move(curr), args);
    }





    template<class T, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::vector<T>, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }

    template<class T, class Compare, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::set<T, Compare>, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }

    template<class T, class Hash, class Eq, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::unordered_set<T, Hash, Eq>, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }


    template<class T, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::vector<T>, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<Vector, ArgDefinition, Annotations...>(member) };
        spec.metavars = std::string{ spec.metavars } + "...";
        return spec;
    }

    template<class T, class Compare, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::set<T, Compare>, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<Set, ArgDefinition, Annotations...>(member) };
        spec.metavars = std::string{ spec.metavars } + "...";
        return spec;
    }

    template<class T, class Hash, class Eq, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::unordered_set<T, Hash, Eq>, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<UnorderedSet, ArgDefinition, Annotations...>(member) };
        spec.metavars = std::string{ spec.metavars } + "...";
        return spec;
    }
}
