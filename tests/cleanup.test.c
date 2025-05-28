#include "../threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>

static void __ucl_test1(void *data) {
    printf("Test function 1 output: %s\n", ((data) ? (char*)data : "<null>") );
    return;
}
static void __ucl_test2(void *data) {
    printf("Test function 2 output: %s\n", ((data) ? (char*)data : "<null>") );
    return;
}

int main(void) {
    cleanup_CREATE(3);
    cleanup_REGISTER(__ucl_test1, "testing 1");
    cleanup_REGISTER(__ucl_test2, "testing 2");
    cleanup_REGISTER(free, strdup("dummy data"));

    cleanup_FIRE();
    printf("\n");

    // Checking to see that you can't push too many objects to the stack
    cleanup_REGISTER(__ucl_test2, "size testing");
    cleanup_REGISTER(__ucl_test1, "size testing");
    cleanup_REGISTER(__ucl_test2, "size testing");
    if(cleanup_REGISTER(__ucl_test1, "size testing") != -1 && errno != EINVAL) error(1, ENOMEM, "cleanup_register didn't error out when it should have");
    
    cleanup_FIRE();
    printf("\n");

    // Make sure the conditional versions work
    cleanup_UNMARK();
    int val = 0;

    cleanup_CNDREGISTER(__ucl_test1, "before flag (should be visible)");
    cleanup_CNDFIRE(); // Shouldn't fire
    cleanup_CNDEXEC(val++;) // Should fire, val = 1;

    cleanup_MARK();
    cleanup_CNDREGISTER(__ucl_test2, "after flag (should NOT be visible)"); // Shouldn't be added to stack
    cleanup_CNDEXEC(val++;) // Shouldn't fire, val = 1;
    cleanup_CNDFIRE(); // Should fire

    printf("val: %d (should be 1)\n\n", val);

    return 0;
}