# Policy for `result` Arguments

## Overview

In XER, public APIs should, as a rule, **not** take `xer::result` as a function argument.

Previously, functions that directly accepted `xer::result` were also allowed, with the intention of making error propagation easier to write.
However, it became clear that this policy could work against the clarity and usability of public APIs.

In particular, when overloads that accept ordinary values coexist with overloads that accept `xer::result`, type deduction and overload resolution can become unnatural at the call site.
As a result, behavior may become harder for users to predict, such as string literals or temporary objects not being accepted naturally.

For this reason, XER limits the use of `xer::result` arguments and aims to make public APIs simpler and easier to understand.

---

## Basic Policy

### General Rule

Ordinary public APIs do not take `xer::result` as an argument.

A function should accept ordinary values and process them as such.
If the caller already holds a `xer::result`, the caller should explicitly extract the success value before passing it to the next function.

### Purpose

The main purposes of this policy are as follows:

- to improve the clarity of public APIs
- to avoid overload conflicts and ambiguity
- to allow string literals and temporary objects to be passed naturally
- to make it easier for users to understand intuitively what kinds of arguments a function accepts
- to simplify documentation and code examples

---

## Exceptions

### `xer/arithmetic.h`

Functions that belong to `xer/arithmetic.h` may, as an exception, accept `xer::result` arguments.

This is because arithmetic and comparison helpers have a clear need to allow multi-step computation results to be chained directly.
In particular, the requirement to propagate errors that occur in intermediate arithmetic steps has substantial design value in this area.

Accordingly, allowing `xer::result` arguments is, as a rule, limited to `xer/arithmetic.h`.

### Functions Whose Subject Matter Is `xer::result` Itself

This policy does not apply to functions whose purpose is to operate on `xer::result` itself.

For example, functions that inspect the success or failure of a `xer::result`, or functions that transform, aggregate, or otherwise assist in handling `xer::result`, may of course take `xer::result` as an argument.

These are fundamentally different from cases where an ordinary public API merely accepts `xer::result` as a convenience.

---

## Design Considerations

### Public APIs Should Accept Ordinary Values

Ordinary public APIs should prioritize the most natural form that users are likely to write.

For example, a string-processing function should naturally accept inputs such as `std::u8string_view` or values equivalent to `const char8_t*`.
If a convenience overload that accepts `xer::result<std::u8string_view>` is added there, usability may become worse.

For that reason, public APIs should prioritize natural handling of ordinary inputs over convenience for error propagation.

### Error Propagation Should Be Explicit at the Call Site

If the caller holds a `xer::result`, the caller should explicitly extract the success value before passing it to the next function.

This may increase the amount of code slightly in some cases, but it makes it clearer at which point errors are checked and which value is being passed onward.
As a result, the intent of the code becomes easier to read, and debugging and documentation also become easier.

### Only Areas Where Chaining Is Essential Should Be Exceptions

Areas such as arithmetic, where chaining and propagation are themselves part of the value of the API, may exceptionally allow `xer::result` arguments.

However, this exception should remain limited.
It should not be expanded to APIs such as string processing, path handling, input/output, time handling, or JSON processing, where accepting ordinary values is the natural design.

---

## Treatment of Existing APIs

Among existing public APIs, any functions outside `xer/arithmetic.h` that currently accept `xer::result` as an argument should be deprecated or removed step by step.

If necessary, temporary migration measures may be introduced with compatibility in mind.
However, because XER is still in the alpha stage, unnecessary compatibility layers should be avoided as much as possible.

---

## Policy for Future APIs

Functions added in the future should also, as a rule, not take `xer::result` as an argument.

When designing a new API, the default should be a form that accepts only ordinary values, and exceptions should be considered only when they are truly necessary.

In this context, a reason such as “it would be convenient to write” is not sufficient for adopting `xer::result` arguments.
If they are adopted, it should be a necessary condition that chained error propagation has essential value as part of the public design of that area.

---

## Effect on Documentation and Code Examples

Code examples and the reference manual should basically show functions being called with ordinary values.

This makes code examples easier for users to imitate directly.
It also makes accepted argument types easier to describe simply in API documentation.

The handling of `xer::result` should be explained not as part of ordinary API argument specifications, but as the basic mechanism for error handling.

---

## Summary

- ordinary public APIs do not take `xer::result` as an argument
- allowing `xer::result` arguments is, as a rule, limited to `xer/arithmetic.h`
- functions whose purpose is to operate on `xer::result` itself are outside the scope of this policy
- among existing public APIs, those outside `xer/arithmetic.h` that accept `xer::result` should be cleaned up step by step
- future functions should also, as a rule, not take `xer::result` arguments
- public documentation and code examples should primarily present the ordinary-value form of APIs
