@page type-parser TypeParser structure

# TypeParser

`TypeParser<T, ArgDefinition>` is the extension point of Ogma, the same way `std::formatter` is the extension point
of `std::format`. Specialize it to teach Ogma how to parse a type from command line tokens, and everything else
follows. Help rendering, error messages, positional support, all of it reads from the same contract.

## The contract

Every specialization provides three static templates.

```cpp
template<class T, Ogma::Arg ArgDefinition>
struct TypeParser {
    template<auto... Annotations>
    static Ogma::Result<T> Parse(T curr, std::string_view flag, Ogma::ArgSpan args,
                                 std::string_view scopePath = {});

    template<auto... Annotations>
    static constexpr Ogma::ArgSpan ExpectedFlags();

    template<auto... Annotations>
    static consteval Ogma::ValueSpec Spec(std::meta::info member);
};
```

`T` is the type being parsed. `ArgDefinition` is the annotation that describes the option, like the
`Arg{ "numbers" }` in `[[ =Ogma::Arg{ "numbers" } ]]`. Every function also receives an `Annotations...` pack
carrying the remaining annotations of the option, so behaviour can be specialized per annotation, like
`[[ =Ogma::Arg{ "config" }, =Ogma::mustExist, =Ogma::fileOnly ]]`.

## Parse

`Parse` is responsible for the actual parsing. It receives the flag used to match this option (if applicable), like
`--cool-flag`, and the remaining arguments.

The `curr` field is the old living object, passed by value. It exists because an argument can pass through `Parse`
multiple times. Numeric types and chrono types just ignore the old value, containers append to it when called
repeatedly. For that relation of conditional append versus override, see `Overwrite` and `Append`.

The `scopePath` parameter is help plumbing, threaded down so nested scope pages know where they are. Leaf
specializations can keep the three-argument signature and ignore it, the struct-walking engine is the one that
propagates it.

## ExpectedFlags

Returns every flag spelling this option answers to, without the `--` prefix. In Ogma an option can answer to
multiple spellings, but normally you need just `{ "", ArgDefinition.longName }`.

The empty spelling is the positional marker. Its presence is what makes a type positional-capable, meaning a bare
token can fill it. A raw bool never contains `""` (its two spellings answer different values, a bare token could not
tell them apart), which is why booleans are named-only.

## Spec

Reports how the member shows up in the help page. It never affects parsing, it exists only so the page can show what
the parser actually accepts. The `member` parameter carries the reflection of the data member being documented,
which lets a specialization derive semantic metavars with `std::meta::identifier_of`, or tell a declared default
apart from a value-initialised field with `std::meta::has_default_member_initializer`.

It has four fields. `usageFlag` is the flag shown in the usage line. `flagList` is every flag that can match, shown
in the row. `metavars` is the value placeholder, self-delimited (`<NAME>`, `{A|B|C}`, `<KEY=VALUE>...`), empty for
members that take no value. `modifiers` is the annotation summary appended after the member's description
(`[format: hex]`, `[range: 0..100]`).

A specialization that changes what it accepts per annotation, like the bool modes selected by `BoolArg`, should
change `Spec` per annotation too. The page must stay in sync with what parsing accepts.

## How the engine finds your specialization

Ogma always consults `TypeParser<FieldT, arg>::Parse` with the member's own annotations substituted into the pack.
If no specialization exists, the primary template runs, and the primary template IS the struct-walking engine. That
is why an annotated struct member recurses automatically, a struct type reaches the primary fallback and walks its
own members, at any depth, root or nested alike.

## The one toolchain caveat

Functions entered through a splice (`std::invoke([: f :])`) must have reflection-free bodies on the current
toolchain, direct consteval calls may reflect. This is why some delegation in the library looks like it is done
twice, the help renderer unwraps `std::optional` members itself instead of calling through the wrapper. Keep that in
mind if you delegate between specializations.