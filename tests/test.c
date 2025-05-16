#include "../source/vector.h"

#define CTF_TEST_NAMES
#include "C-Testing-Framework/ctf.h"

#include <stdlib.h>

Allocator a = {malloc,realloc,free};

TEST_MAKE(InitFree)
{
    int *v = vector(int, &a);
    TEST_ASSERT(v != NULL);
    TEST_ASSERT(vector_free(v) == 0);
    TEST_PASS();
}

TEST_MAKE(Append)
{
    int *v = vector(int, &a);
    vector_push_back(v, 10);
    vector_push_back(v, 20);
    vector_push_back(v, 30);
    size_t len;
    TEST_ASSERT(vector_get_len(v, &len) == 0);
    TEST_ASSERT(len == 3);
    TEST_ASSERT(v[0] == 10);
    TEST_ASSERT(v[1] == 20);
    TEST_ASSERT(v[2] == 30);
    vector_free(v);
    TEST_PASS();
}

TEST_MAKE(PopBack)
{
    int *v = vector(int, &a);
    vector_push_back(v, 100);
    vector_push_back(v, 200);
    vector_push_back(v, 300);
    int popped;
    vector_pop_back(v, &popped);
    TEST_ASSERT(popped == 300);
    size_t len;
    vector_get_len(v, &len);
    TEST_ASSERT(len == 2);
    vector_pop_back(v, &popped);
    TEST_ASSERT(popped == 200);
    vector_get_len(v, &len);
    TEST_ASSERT(len == 1);
    vector_free(v);
    TEST_PASS();
}

TEST_SUITE(Vector,
{
    TEST_SUITE_LINK(Vector,InitFree);
    TEST_SUITE_LINK(Vector,Append);
    TEST_SUITE_LINK(Vector,PopBack);
})

int main(int argc, char** argv)
{
    TEST_PROCESS_INIT();
    TEST_SUITE_RUN(Vector);
    TEST_PROCESS_EXIT();
}