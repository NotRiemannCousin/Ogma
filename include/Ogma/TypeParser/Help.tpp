#pragma once

//! @file Ogma/TypeParser/Help.tpp
//! @brief Help-page rendering over the reflection layer.

#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Ogma{ template<class T, Arg ArgDefinition> struct TypeParser; }

namespace Ogma::details_ {

    struct HelpEntry {
        ConstString spelling{};
        ConstString description{};
    };

    struct HelpPageData {
        std::string_view aboutLine{};
        std::string_view usageSuffix{};
        std::span<const HelpEntry> commands{};
        std::span<const HelpEntry> arguments{};
        std::span<const HelpEntry> options{};
        std::span<const HelpEntry> other{};
    };

    template<class T>
    consteval HelpPageData HelpDataOf() {
        namespace mt = std::meta;
        namespace vs = std::views;
        using namespace std::string_view_literals;
        using VecStrView = std::vector<std::string_view>;

        static constexpr auto args{ std::define_static_array(Utils::MatchedMembers<^^Arg>(^^T)) };
        static constexpr auto subcommand{ std::define_static_array(Utils::MatchedMembers<^^VariantArg>(^^T)) };

        std::vector<std::string> reqUsageFrags{}; // required
        std::vector<std::string> optUsageFrags{}; // optional
        std::vector<std::string> cmdUsageFrags{};
        std::vector<HelpEntry> commands{};
        std::vector<HelpEntry> arguments{};
        std::vector<HelpEntry> options{};
        std::vector<HelpEntry> other{};

        std::string aboutLine{};
        if constexpr (HasAppInfo<T>()) {
            const auto [name, version, description]{ mt::extract<AppInfo>(std::meta::constant_of(mt::annotations_of_with_type(^^T, ^^AppInfo)[0])) };

            const std::string_view appName{ name.empty() ? mt::identifier_of(^^T) : std::string_view{ name } };
            aboutLine = description.empty()
                ? VecStrView{ appName, version }                   | Utils::concatBySpace
                : VecStrView{ appName, version, "-", description } | Utils::concatBySpace;
        }

        template for (constexpr auto entry : args) {
            constexpr auto member{ entry.first };
            constexpr auto annotation{ entry.second };
            constexpr Arg arg{ mt::extract<Arg>(annotation) };
            if (arg.hidden) continue;

            using FieldT = TypeOf<member>;

            constexpr auto annots{ std::define_static_array(Utils::AnnotationValues(member)) };
            constexpr bool isOptional{ Utils::IsSpecializationOf<FieldT>(^^std::optional) };
            constexpr bool isNamedOnly{ IsNamedOnly<FieldT>(annots) };

            // just in case
            static_assert(!isOptional || isNamedOnly, "std::optional members are named-only by contract");

            constexpr auto specFn{ mt::substitute(^^TypeParser<FieldT, arg>::Spec, annots) };
            constexpr ValueSpec spec{ std::invoke([: specFn :], member) };

            auto usageStr{ VecStrView{ spec.usageFlag, spec.metavars } | Utils::concatBySpace };

            auto row { VecStrView{ spec.flagList  , spec.metavars  } | Utils::concatBySpace };
            auto desc{ VecStrView{ arg.description, spec.modifiers } | Utils::concatBySpace };


            if constexpr (isOptional      ) usageStr = "[" + usageStr + "]", row = "[" + row + "]";
            else if constexpr (isNamedOnly) usageStr = "(" + usageStr + ")";

            auto& usageContainer{ isOptional  ? optUsageFrags : reqUsageFrags };
            auto& infoContainer { isNamedOnly ? options       : arguments     };

            infoContainer.push_back(HelpEntry{ row, desc });
            usageContainer.push_back(usageStr);
        }

        template for (constexpr auto entry : subcommand) {
            constexpr auto annotation{ entry.second };

            constexpr VariantArg variant{ mt::extract<VariantArg>(annotation) };

            cmdUsageFrags.push_back("<COMMAND>");
            for (auto [longName, description] : vs::counted(variant.cases, variant.caseCount))
                commands.push_back(HelpEntry{ longName.ptr, description.ptr });
        }

        if constexpr (HasAppInfo<T>()) other.push_back(HelpEntry{ "--version", "Print version" });

        if (optUsageFrags.size() > 5) optUsageFrags = { "[OPTIONS]" };

        other.push_back(HelpEntry{ "--help", "Print help" });
        other.push_back(HelpEntry{ "-", "Close the current scope; remaining arguments belong to the parent" });

        return HelpPageData{
            .aboutLine   = std::define_static_string(aboutLine),
            .usageSuffix = std::define_static_string(vs::concat(reqUsageFrags, optUsageFrags, cmdUsageFrags) | Utils::joinBySpace),
            .commands    = std::define_static_array(commands),
            .arguments   = std::define_static_array(arguments),
            .options     = std::define_static_array(options),
            .other       = std::define_static_array(other),
        };
    }

}

namespace Ogma {

    template<class T>
    std::string RenderHelpPage(const std::string_view scopePath) {
        using namespace details_;
        namespace vs = std::views;
        namespace rg = std::ranges;

        static constexpr HelpPageData data{ HelpDataOf<T>() };
        using VecStrView = std::vector<std::string_view>;

        std::string page;
        if (!data.aboutLine.empty()) {
            page += data.aboutLine;
            page += "\n\n";
        }

        page += "Usage: " + (VecStrView{ scopePath, data.usageSuffix } | Utils::concatBySpace) + "\n\n";


        const auto emit{ [&](const std::string_view header, const std::span<const HelpEntry> rows) {
            if (rows.empty()) return;

            page += header;
            page += '\n';

            const std::size_t width{ 2 + rg::max(rows
                    | vs::transform(&HelpEntry::spelling)
                    | vs::transform(&ConstString::size)
            ) };

            auto iter{ std::back_inserter(page) };
            for (const auto& [spelling, desc] : rows)
                if (desc.empty()) std::format_to(iter, " {}\n"      , spelling);
                else              std::format_to(iter, " {:<{}}{}\n", spelling, width, desc);

            page += '\n';
        } };

        emit("Commands:"            , data.commands);
        emit("Positional arguments:", data.arguments);
        emit("Named options:"       , data.options);
        emit("Other:"               , data.other);
        return page;
    }



    template<class T>
    std::string HelpText(const std::string_view scopePath) {
        return RenderHelpPage<T>(
            scopePath.empty() ? ScopePathOf<T>() : scopePath);
    }

}