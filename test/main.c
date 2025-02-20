#include "../src/lib/memory.h"
#include "../src/lib/string.h"
#include "../src/lib/assert.h"
#include "../src/lib/print.h"

void test_strcpy() {
    char dest[20];
    char *src = "Hello, world!";
    strcpy(src, dest);
    assert(strcmp(dest, src) == 0, "test_strcpy 1");
    printfmt("test_strcpy passed!\n");
}

void test_strcmp() {
    assert(strcmp("apple", "apple") == 0, "test_strcmp 1");
    assert(strcmp("apple", "banana") == -1, "test_strcmp 2");
    printfmt("test_strcmp passed!\n");
}

void test_memset() {
    char buffer[10] = {0};
    memset(buffer, 'A', sizeof(buffer));
    for (int i = 0; i < 10; i++) {
        assert(buffer[i] == 'A', "test_memset 1");
    }
    printfmt("test_memset passed!\n");
}

int run_tests() {
    test_strcmp();
    test_strcpy();
    test_memset();
    printfmt("All tests passed!\n");
    return 0;
}
