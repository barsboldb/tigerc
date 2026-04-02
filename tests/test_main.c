#include "test.h"
#include <stdio.h>

test_entry_t test_registry[MAX_TESTS];
int          test_count   = 0;

int main() {
    int passed = 0;
    int failed = 0;

    for (int i = 0; i < test_count; i++) {
        if (test_registry[i].fn()) {
            printf(GREEN "PASS" RESET ": %s\n", test_registry[i].name);
            passed++;
        } else {
            printf(RED "FAIL" RESET ": %s\n", test_registry[i].name);
            failed++;
        }
    }

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
