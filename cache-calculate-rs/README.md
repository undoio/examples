# cache-calculate-rs

A rewrite of `cache.c` in Rust, intended to be used as a simple first demo of UDB.

## Debugging Instructions

### Getting Started

1. Build the project with `cargo build`
  * Rust would normally catch integer overflows required for this bug to work, but I've disabled them in `Cargo.toml`.
  * Release builds are more annoying to debug
2. Debug the program with `udb ./target/debug/cache-calculate-rs`

### Triggering the Bug

1. Run `start` to load the program
2. Run `continue` to run the program. The program should panic quickly.
3. Set a temporary breakpoint on Rust's panic function by running `tb core::panicking::panic`
  * When the program panics it leaves us in the weird teardown part of a program
  * We need to get back to Rust land
4. Run `reverse-continue` to get to the panic
5. Run `up` to get out of the panic handler

At this point, you should be on the `assert!` line that is making the program panic.

```
end 621,037> tb core::panicking::panic
Temporary breakpoint 2 at 0x5555555e9d28: file library/core/src/panicking.rs, line 145.                                                                                                                                                      
end 621,037> rc                                                                                                                                                                                                                             
Continuing.

Temporary breakpoint 2, core::panicking::panic () at library/core/src/panicking.rs:145
99% 617,286> up
#1  0x00005555555717fc in cache_calculate_rs::main () at src/main.rs:55
55              assert!(sqroot_cache == sqroot_correct);
99% 617,286> 
```

### Going Back to the Root Cause

(From this point it's useful to be in `layout dashboard`)

From here we can see the incorrect square root results in the Locals view. It may be worth adding a bookmark here.

1. Use `reverse-next` to step up to the line before we run `cache_calculate()`
  * Specifically, the line where we calculate `sqroot_correct`
  * This is because we want to start looking at the end of `cache_calculate()`
2. Use `reverse-step` to go into our bad `cache_calculate()`
3. Reverse step again to see that we're in the cached case
4. We need to find out why our cache has an incorrect value here. To do this, we can run `last entry.sqroot`.
5. From here, we can see exactly where the bug occured. Since `number_adj`'s for loop doesn't have any range checks, we end up overflowing and calculating either the square root of -1, or the square root of 256, which overflows to 0 when stored as a `u8`.
