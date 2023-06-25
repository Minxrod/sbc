#pragma once

// behold, "tests"
// Expect true
#define ASSERT(a, msg) if (!(a)) { printf("Failed: "msg"\n"); return 1; }
// Expect false
#define DENY(a, msg) if (a) { printf("Failed: "msg"\n"); return 1; }

