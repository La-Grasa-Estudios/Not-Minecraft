#pragma once

#define require(x) if (!x) { printf("Fatal Error: %s failed\n", #x); } else { printf("%s success\n", #x); }