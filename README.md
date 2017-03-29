# FastStack

An in progress rewrite in Rust.

## Why a rewrite?

 - Easier cross-platform building
 - Less header-dependency maintenance
 - Stronger type-system
 - Want to improve a number of areas lacking in the previous design
 - Want to explore a `no_std` engine based on the current C code

We lose some of the portability of C, but realistically a C FFI could be
constructed and the base engine written with `no_std` support would
rival the C implementation in size.
