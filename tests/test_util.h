#pragma once

// behold, "tests"
// Expect true
#define ASSERT(a, msg) if (!(a)) { printf("\033[31mFailed: "msg"\033[0m\n"); return 1; }
// Expect false
#define DENY(a, msg) if (a) { printf("\033[31mFailed: "msg"\033[0m\n"); return 1; }
// Report all tests passed
#define SUCCESS(msg) printf("\033[32mSuccess: "msg"\033[0m\n"); return 0;
