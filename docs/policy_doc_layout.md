# Documentation Asset Placement Policy

## Overview

XER places reference manuals, policy documents, operational documents, code examples, and development helper scripts in different locations according to their roles.

If these materials are placed without clear structure, the following problems are likely to occur:

- user-facing documents and developer-facing documents become mixed together
- it becomes unclear which file is the canonical source
- assumptions for automatic generation and extraction become fragile
- future reorganization and relocation become unnecessarily difficult

For this reason, XER separates documents and auxiliary assets according to their roles.

This policy document primarily defines placement rules for the following locations:

- `docs/`
- `docs/bits/`
- `examples/`
- `php/`

---

## Basic Policy

### Separate user-facing and developer-facing materials

Documents intended for direct reading by users and documents intended for design, maintenance, and automation by developers serve different purposes.

Accordingly, materials should be organized at least into the following categories:

- user-facing documents
- developer-facing auxiliary documents
- executable code examples
- development-only scripts

### Make canonical sources explicit

Do not duplicate the same content across multiple locations unless there is a clear reason.

For each piece of content, make the canonical source as explicit as possible.
Even when other files quote, summarize, or generate from it, the canonical source must remain unambiguous.

### Place files by role

A file should be placed according to its role, not according to its extension or personal preference.

---

## Role of `docs/`

### Basic position

`docs/` is the primary directory for XER documentation.

This directory should contain documents that satisfy at least one of the following:

- policy documents that affect the project as a whole
- user-facing documents
- documents that are also useful to developers and still carry project-wide meaning
- documents that may become part of published documentation in the future

### What should be placed directly under `docs/`

The following kinds of documents should be placed directly under `docs/`:

- the overall project outline
- coding conventions
- project-wide policy documents for I/O, text encoding, path handling, time handling, and similar topics
- cross-cutting design rules such as the handling of `xer::result` arguments
- the code example policy
- this documentation asset placement policy
- entry-point documents for the reference manual or indexes

### Why these documents belong directly under `docs/`

These documents are not limited to one specific header or one isolated feature.
They affect XER as a whole.

For that reason, they should remain easy to find directly under `docs/` rather than being buried in deeper hierarchy.

---

## Role of `docs/bits/`

### Basic position

`docs/bits/` is the place for smaller supporting documents that help build or organize the main documentation.

It may contain documents such as the following:

- supplementary notes for specific topics
- fragment documents intended for future generation or reorganization
- documentation pieces used as inputs to generation scripts
- supporting fragments used to construct a specific published document
- detailed notes that are too fine-grained to place directly under `docs/`

### What may be placed in `docs/bits/`

At minimum, the following kinds of files may be placed in `docs/bits/`:

- reference manual fragments
- tutorial fragments
- intermediate documents read by generation scripts
- supporting fragments used to build a specific published document
- detailed supplementary notes that should not appear directly under `docs/`

### What should not be placed in `docs/bits/`

As a rule, the following should not be placed in `docs/bits/`:

- the fundamental project-wide policies
- coding conventions
- design principles that affect the public API as a whole
- major documents that users should read directly
- documents that should be explicitly treated as canonical primary documents

### Positioning

`docs/bits/` plays a role similar to `xer/bits/` in the sense that both are for subdivision, support, and internal organization.

However, unlike `xer/bits/`, which is for implementation detail code, `docs/bits/` is for documentation organization.
A file under `docs/bits/` is not forbidden to users; it is simply not the main entry point by default.

### Flat structure

`docs/bits/` should remain flat.

If classification is needed, it should be expressed by file naming conventions such as prefixes rather than by creating subdirectories under `docs/bits/`.

Examples of such prefixes include:

- `policy_`
- `header_`
- `snippet_`

This keeps the structure simple and makes it easier for scripts to collect related files mechanically.

---

## Role of `examples/`

### Basic position

`examples/` is the directory for user-facing code examples.

A code example is documentation, but it is not merely explanatory prose.
It is actual source code that can be compiled and run.

For that reason, code examples should be managed separately from ordinary Markdown documents.

### What should be placed in `examples/`

`examples/` should contain `.cpp` files that satisfy at least the following conditions:

- they can be compiled independently
- they can be run independently
- they show natural usage of the public API
- they contain `XER_EXAMPLE_BEGIN` / `XER_EXAMPLE_END`
- they may contain human-oriented comments such as `Expected output:`

### Why `examples/` is separated

Code examples differ in nature from policy documents and reference-manual prose:

- they are compilation targets
- they are execution targets
- they may later be extracted or republished into documentation
- readability of the code itself matters in addition to the comments

For these reasons, they should live in a dedicated directory separate from ordinary documents.

### Canonical position of examples

As a long-term goal, code examples shown in documentation should be based on files under `examples/` whenever possible.

In other words, examples in the reference manual or tutorials should preferably come from actual examples that have been compiled and run, rather than from hand-written one-off snippets embedded only in the document text.

---

## Role of `php/`

### Basic position

`php/` is the place for PHP scripts used as development-only tools in XER.

It may contain at least the following kinds of files:

- code generation scripts
- test helper scripts
- compile/run control scripts
- result aggregation scripts
- auxiliary data needed by those scripts

### What may be placed in `php/`

Examples include:

- `run_tests.php`
- conversion-table generation scripts
- test-case generation scripts
- documentation generation helper scripts
- templates and auxiliary data used by those scripts

### What should not be placed in `php/`

As a rule, the following should not be placed in `php/`:

- major user-facing documents
- canonical code examples
- policy documents mainly intended for human reading
- public headers of XER itself

### Positioning

`php/` is not part of the XER library itself.
It is a dedicated area for development-time support.

Accordingly, XER must not require end users to use PHP merely to use the library.

---

## Classification Rules for Documents

If there is uncertainty about where a file should be placed, classify it in the following order.

### 1. Is it an executable code example?

If it is a user-facing code example that can be compiled and run, place it in `examples/`.

### 2. Is it a development-only script?

If it is a PHP script for automation, generation, or compile/run control, place it in `php/`.

### 3. Is it a major project-wide document?

If it is a project-wide design policy, convention, or entry-point document, place it directly under `docs/`.

### 4. Is it a supporting or fragmentary document?

If it is not itself a major document, but rather a supporting fragment, generation part, or detailed supplement, place it in `docs/bits/`.

---

## Policy on Canonical Sources

### Canonical source of primary documents

Primary documents such as design policies and conventions should, as a rule, live under `docs/` and be treated as canonical there.

Even if they are transformed into another format or summarized elsewhere, the original document under `docs/` remains the canonical source.

### Canonical source of code examples

Code examples should, as a rule, use the `.cpp` files under `examples/` as their canonical source.

If short code snippets are shown in documentation, they should ideally be extracted from those files.

### Treatment of generated files

Intermediate files and outputs generated by PHP scripts may be committed when appropriate, but the distinction between source files and generated files must remain clear.

Whenever a generation rule exists, it should remain easy to understand which files are inputs and which files are generated outputs.

---

## Naming Policy

### Names of primary documents

Primary policy and convention documents should use English file names.

They should be descriptive, stable, and easy to understand from the file name alone.

Examples:

- `policy_project_outline.md`
- `coding_conventions.md`
- `policy_result_arguments.md`
- `policy_examples.md`
- `policy_doc_layout.md`

### Names of supporting fragments

Files under `docs/bits/` should use English lower snake case names.

When needed, classification should be expressed by prefixes in the file name rather than by additional subdirectories.

Examples:

- `policy_encoding.md`
- `header_stdio.md`
- `snippet_result_basics.md`

Naming should remain consistent within the directory.

---

## Language of Documentation and Comments

### Source code comments

Comments in source code should be written in English.

### Documentation files

Policy documents, design documents, and user-facing documentation should also be written in English.

English documentation is treated as the canonical source.
Older documents in other languages may remain in Git history, but the repository should not require ongoing parallel maintenance of multiple language versions for the same policy document.

---

## Relationship to the Reference Manual

### Basic policy

The reference manual is a major user-facing document and therefore belongs on the `docs/` side.

### Use of supporting fragments

If the reference manual is later modularized or generated automatically, its fragments and supporting inputs may be placed in `docs/bits/`.

Even in that case, the position of the completed manual as the published document must remain clear.

### Relationship with code examples

Code examples shown in the reference manual should, whenever possible, be quoted from or derived from files under `examples/`.

This makes it easier to reflect examples that have actually been built and run into the documentation.

---

## Policy for Future Expansion

If the number of documents or examples increases, the following extensions may be introduced when necessary:

- category-based subdirectories under `examples/`
- separation of generation scripts and test scripts under `php/`
- additional rules for automatic extraction and embedding

However, the initial structure should remain as simple as possible, with only the minimum necessary directories.

In particular, `docs/bits/` should remain flat even if the number of fragment files grows.
Classification inside that directory should continue to rely on naming conventions.

---

## Concrete Placement Examples

### `policy_project_outline.md`

The project-wide outline belongs directly under `docs/` because it defines the basic design of the project as a whole.

### `policy_result_arguments.md`

A cross-cutting API design rule belongs directly under `docs/`.

### `policy_examples.md`

A major document that defines the operating rules for `examples/` belongs directly under `docs/`.

### `example_trim.cpp`

An independently compilable and runnable code example belongs in `examples/`.

### `run_tests.php`

A development-time compile/run control script belongs in `php/`.

### Reference manual fragments

If a file is not the completed published document itself but rather a fragment used for generation or organization, it may be placed in `docs/bits/`.

---

## Summary

- `docs/` is the place for major project-wide documents
- `docs/bits/` is the place for supporting fragments, generation parts, and detailed supplements
- `docs/bits/` should remain flat, and classification within it should be done by file naming conventions
- `examples/` is the place for independently compilable and runnable code examples
- `php/` is the place for development-only PHP scripts and auxiliary data
- placement is decided by role, not by extension
- primary policy and convention documents should normally be canonical under `docs/`
- code examples should normally be canonical under `examples/`
- even when using automatic generation or quotation, the canonical source must remain explicit
- the initial structure should remain minimal and should avoid unnecessary subdivision
