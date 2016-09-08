HACKING
=======

This file provides some rough outlines on how to go about doing common tasks
within the codebase.

Adding a new option
-------------------

Adding an option is fairly straight forward. Besides the actual implementation
of the option, the following steps will always need to be performed.

 * Add an entry to `FSGame` in `fs.h`
 * Add a compile-time default value to `fsDefault.h`
 * Set the value to its default in `fsGameInit` in `fs.c`
 * Add a check for the value in `unpackOptionValue` in `fsOption.c`

Adding a new frontend
---------------------

A frontend is a large task, but the following things need to be performed.

 * Declare a `FSPSView` definition with require frontend specific data.
 * Implement every `fsi` prefixed function in `fsInterface.h`
 * Define a `const char *fsiFrontendName` variable within your code. This does
   not need to be externed explicitly (done within the engine).
