@page scopes Scopes

# Scopes

Ogma does not parse a flat list of tokens. It parses a tree of scopes, and the closing token `-` is what moves you
between them. This page explains what a scope is, how you enter and leave one, and why the rules are stricter here
than in other parsers.

## What counts as a scope

Every annotated struct member opens its own scope. A nested struct opened by its flag, a container of named-only
values, and a variant-backed subcommand all behave the same way once entered. The branch owns every token that
follows it, its own flags answer, its own positional members fill, and its nested members open further scopes.

There is no "global" scope that all members share. Each struct level owns exactly the members it declares.

## Entering and leaving

You enter a scope by typing the selector of one of its members. For a nested struct that is its flag (`--inner`),
for a subcommand that is the bare case name (`compress`), for a container element of named-only values that is the
control token `--`.

You leave a scope with the closing token `-`. It exits exactly one level of nesting, no more, and hands the
remaining arguments back to the parent scope. The parent then continues parsing where it stopped, with its own flags
and its own unfilled positional members.

```text
app --verbose inner --name Ada - --tag hello
```

Here `--verbose` and `--tag` belong to `app`, `--name` belongs to `inner`, and the `-` is what draws that line. The
closing token is optional whenever the argument list ends there anyway, and a trailing one at the root is harmless.

A trailing `-` followed by more arguments at the root is an error, because at the true top there is no parent to hand
anything to. `Ogma::Parse` reports the leftover as an unexpected argument.

## Flags never cascade

Flags of the parent are read before the branch opens. After it, they are unknown inside the branch, and flags of the
branch are unknown to the parent. Nothing cascades in either direction.

```text
app --verbose compress x.tar     # --verbose belongs to app, read before the branch
app compress x.tar --verbose     # error, --verbose is unknown inside compress
```

An unrecognized flag is an error of the scope you are in, which is what keeps error messages honest about where you
are. The same segregation applies to positional members, a bare token fills the first unfilled positional of the
current scope, never one from another level.

## Homonyms are fine

Parent and child can both declare a member with the same long name. Each occurrence answers to its own level, and
the two members never learn about each other.

```text
app --name outer inner --name inner
```

This is a direct consequence of scopes never merging. Flattening parsers need namespaces or prefixes to express
this, Ogma needs nothing, the boundary is structural.

## The closing token in repeated structural containers

The same boundary that separates scopes also separates elements. A `vector<Struct>` member parses one element per
group, each closed by its own `-`, and omitting the boundary between two elements reads as more options for the
current element, not as the start of a new one.

```text
--points -- --x 1 --y 2 - --points -- --x 3 --y 4 -
```

The `-` is what draws the line, not position. For containers of positional types (like `vector<string>`) no explicit
opening is needed, the container just consumes tokens until it hits a flag or a closing token, and for containers of
named-only types the control token `--` opens each new element.

## `-` and `--` are different tokens

`-` is the closing token, it exits one scope. `--` is not an end-of-options delimiter here, the POSIX meaning was
repurposed. It opts out when the next value can be misunderstood as a flag (`-- --base64=idk`), and it opens a new
element of a container of named-only values. Parsing of the surrounding scope continues normally after it, it does
not terminate anything.

The practical consequence is that Ogma has no way to say "everything from here on is positional". If you need a
positional value that starts with `--`, escape just that value with `--`, or fill the member by its flag.
