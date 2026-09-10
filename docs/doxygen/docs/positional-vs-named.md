@page positional-vs-named Positional vs Named Arguments

# Positional vs named

Every annotated member of an Ogma struct answers exactly two questions. Can it be invoked without its flag, by a
bare token? And is it required? The two answers are independent, and this page explains how they work, what the
contract is, and the one deliberate asymmetry in it.

## The two protocol questions

**Can it be invoked positionally?** This is decided by the member's own type, through its `TypeParser::ExpectedFlags`.
If the returned flags contain the empty spelling `""`, the type is positional-capable, a bare token on the command
line can fill it. If not, it is named-only, and you must type its flag.

Strings, numbers, paths, enums, durations and containers are positional-capable. Nested structs, raw booleans and
`std::optional` are named-only. You can check this at compile time with the `IsPositionalConcept` /
`IsNamedOnlyConcept` concepts.

**Is it required?** This is decided exclusively by the wrapper `std::optional`. A member declared as `T` must be
filled, either because a bare token reached it or because its flag was typed. A member declared as
`std::optional<T>` may stay unset, and then it holds whatever it held before parsing (usually `std::nullopt`, which
is how you detect "the flag was not given").

Required does not mean "must be provided by position". A required positional member can perfectly be filled by its
flag instead, and a member filled by flag leaves the positional queue.

## The contract, and why it is one-directional

Omittable implies named-only. An `std::optional` member can never compete for bare tokens, because the queue fills
by position and there is no way to signal "skip this slot". The only way for an optional to stay unset is for its
flag to never appear. `std::optional`'s own `ExpectedFlags` never contains `""`, so this holds by construction.

The reverse does not hold. Named-only does not imply omittable. A nested struct is named-only but required, a
container is positional-capable but required. This is the rule "nobody is special", with `std::optional` as the
single sanctioned exception, being the only type that can be left unset.

```cpp
struct Options {
    [[ =Ogma::Arg{ "query" } ]] std::string query{};             // positional-capable, required
    [[ =Ogma::Arg{ "config" } ]] Config config{};                // named-only, required
    [[ =Ogma::Arg{ "tag" } ]] std::optional<std::string> tag{};  // named-only, omittable
};
```

## How the positional queue works

Bare tokens fill the first still-unfilled positional member of the current scope, in declaration order. A member
already filled by its flag is skipped, and a member that left the queue cannot be claimed again by a later bare
token.

Because the order is semantics, the subcommand member must be the last annotated member of its struct. Bare tokens
would otherwise fill earlier positional members before the command was ever reachable, and a struct with a required
positional declared before the variant would swallow the subcommand's own name. Members that must not compete for
bare tokens go into `std::optional` (filled by name).

## The named table

Flags are matched against a compile-time table built from every member's own `ExpectedFlags` (minus the empty
spelling). The table is sorted and duplicate-free, and two members claiming the same spelling are a compile-time
error. A raw bool contributes two spellings, `--name` and the synthetic `--no-name`, which is why a raw bool can
never be positional, its spellings answer different values and a bare token could not tell them apart.

Named occurrences repeat freely, which is how containers accumulate. Positional members are claimed exactly once.

## Escaping flag-shaped values

A positional string can receive a value that starts with `--` by escaping it with the control token.

```text
--name -- --Green
```

The same control token is what opens each new element of a container of named-only values. It is not an
end-of-options delimiter, parsing of the surrounding scope continues normally after it.

## The help page reflects all of this

The usage line and the sections are a direct rendering of these two answers. Required members appear unbracketed,
`std::optional` members appear bracketed, named-only members appear with their flag, positional members appear as
bare metavariables. Nothing on the page is written by hand, so the page and the parser cannot drift apart. See
[Help](Help.md).
