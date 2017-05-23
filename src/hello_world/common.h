#pragma once

#define memory_error(fmt, ...) do { \
        fprintf(stderr, "%s: %s (%d): not enough memory: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__); \
        exit(1); \
} while (0)