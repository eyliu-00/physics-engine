#include "list.h"
#include "test_util.h"
#include "vector.h"
#include "shapes.h"

#include <assert.h>
#include <stdlib.h>

void test_square() {
    List *square = make_square(10.0);
    assert(list_size(square) == 4);
    assert(vec_equal(*((Vector *)list_get(square, 0)), (Vector) {5.0, 5.0}));
    assert(vec_equal(*((Vector *)list_get(square, 1)), (Vector) {-5.0, 5.0}));
    assert(vec_equal(*((Vector *)list_get(square, 2)), (Vector) {-5.0, -5.0}));
    assert(vec_equal(*((Vector *)list_get(square, 3)), (Vector) {5.0, -5.0}));
    list_free(square);
}

void test_star() {
    List *star = make_n_star(6, 10.0);
    assert(list_size(star) == 12);
}

int main(int argc, char *argv[]) {
    // Run all tests if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_square)
    DO_TEST(test_star)

    return 0;
}
