# Coding Conventions

## Purpose

The purpose of these conventions is to ensure readability, consistency, and maintainability in the XER source code.

These conventions do not attempt to over-document minute formatting rules of the kind often compared to trivial etiquette.
Formatting matters that can be unified mechanically by automatic formatting should, as far as possible, be delegated to `clang-format` and `.editorconfig`.

## Basic Formatting Policy

- The basic formatting style is LLVM style.
- Indentation uses spaces rather than tabs.
- Source files use UTF-8 with BOM.
- Line endings, character encoding, final newlines, trailing whitespace, and similar details are unified by `.editorconfig`.
- Brace placement, insertion of spaces, line length, and similar formatting are unified by `clang-format`.
- As a rule, no separate textual rule is written for matters that can be unified automatically.

## Identifier Naming Rules

- The basic naming style for identifiers is snake case.
- In these conventions, "snake case" means lower snake case.
- Any identifiers for which no separate rule has been defined up to this point also use snake case.
- When reimplementing an existing function from C or PHP, the original name is followed in principle.
- Japanese identifiers are not used.

## Rules for Type Names

- Type names and class names also use snake case.
- Scalar type names use `_t` as a suffix.

## Rules for Macros

- As a rule, macros use uppercase names with the prefix `XER_`.
- However, macros used with function-like syntax, such as `xer_assert`, use lowercase names.

## Rules for Internal Implementation

- Suffixes such as `_impl` are not added to identifiers merely to indicate that they are for internal implementation.
- Identifiers for internal implementation belong to the `xer::detail` namespace.

## Rules for Comments

- Comments are written in English.
- API description comments are written in Doxygen format.
- Doxygen comments use JavaDoc-style notation.
- Comments should be written where intent, assumptions, constraints, or cautions need to be explained, rather than merely restating the code.
- Excessive comments should not be added to self-evident processing.

## Operational Policy

- Formatting variation should be absorbed by automatic formatting as much as possible, rather than by review.
- In the main body of the conventions, priority is given to matters related to XER's design policy and implementation precautions.
- Even when adding or revising a convention, automatic solutions should be considered first.
