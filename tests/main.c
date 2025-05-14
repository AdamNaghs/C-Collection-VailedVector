#define VECTOR_DEBUG
#include "../source/vector.h"

#include <stdio.h>
#include <stdlib.h> /*  for malloc, realloc, free */

/*  -------- Tests ---------- */

int test_init_free(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;
    VECTOR_VALIDATE(vec);
    if (vector_free(vec) != 0)
        return 1;
    return 0;
}

int test_append0(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;

    vector_push_back(vec, 10);
    vector_push_back(vec, 20);
    vector_push_back(vec, 30);

    size_t len;
    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 3)
        return 1;

    if (vec[0] != 10)
        return 1;
    if (vec[1] != 20)
        return 1;
    if (vec[2] != 30)
        return 1;

    vector_free(vec);
    return 0;
}

int test_pop_back(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;

    vector_push_back(vec, 100);
    vector_push_back(vec, 200);
    vector_push_back(vec, 300);

    int *popped = vector_pop_back(vec);
    if (!popped || *popped != 300)
        return 1;

    size_t len;
    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 2)
        return 1;

    popped = vector_pop_back(vec);
    if (!popped || *popped != 200)
        return 1;

    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 1)
        return 1;

    vector_free(vec);
    return 0;
}

int test_resize(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;
    int i;
    for (i = 0; i < 1000; ++i)
    {
        vector_push_back(vec, i);
    }

    size_t len;
    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 1000)
        return 1;

    for (i = 0; i < 1000; ++i)
    {
        if (vec[i] != i)
            return 1;
    }

    vector_free(vec);
    return 0;
}

int test_can_append(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;
    int i;
    for (i = 0; i < VECTOR_DEFAULT_CAP; ++i)
    {
        vector_push_back(vec, i);
    }

    if (vector_can_append(vec) != 0) /*  Should be full */
        return 1;

    vec = vector_resize(vec, VECTOR_DEFAULT_CAP * 2);
    if (!vec)
        return 1;

    if (vector_can_append(vec) != 1) /*  Should have space now */
        return 1;

    vector_free(vec);
    return 0;
}

int test_get_cap(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;

    size_t cap;
    if (vector_get_cap(vec, &cap) != 0)
        return 1;
    if (cap != VECTOR_DEFAULT_CAP)
        return 1;

    vector_free(vec);
    return 0;
}

int test_set_len(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;

    if (vector_set_len(vec, 10) != 0)
        return 1;

    size_t len;
    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 10)
        return 1;

    vector_free(vec);
    return 0;
}

int test_remove(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;

    vector_push_back(vec, 1);
    vector_push_back(vec, 2);
    vector_push_back(vec, 3);

    if (vector_remove(vec, 1) != 0)
        return 1;

    size_t len;
    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 2)
        return 1;

    /*  Unordered remove, so the second element can be anything from original 1 or 3 */
    if (vec[0] != 1 && vec[0] != 3)
        return 1;
    if (vec[1] != 1 && vec[1] != 3)
        return 1;

    vector_free(vec);
    return 0;
}

int test_remove_ordered(void)
{
    Allocator a = {malloc, realloc, free};
    int *vec = vector(int, &a);
    if (!vec)
        return 1;

    vector_push_back(vec, 5);
    vector_push_back(vec, 6);
    vector_push_back(vec, 7);

    if (vector_remove_ordered(vec, 1) != 0)
        return 1;

    size_t len;
    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 2)
        return 1;

    if (vec[0] != 5)
        return 1;
    if (vec[1] != 7)
        return 1;

    vector_free(vec);
    return 0;
}

int test_vector_of_dyn(void)
{
    Allocator a = {malloc, realloc, free};

    typedef int *IntVector;
    IntVector *vec = vector(IntVector, &a);
    if (!vec)
        return 1;

    /*  Create a few inner vectors manually */
    size_t i,j; 
    for (i = 0; i < 3; ++i)
    {
        int *inner = malloc(sizeof(int) * 4);
        if (!inner)
            return 1;
        for (j = 0; j < 4; ++j)
        {
            inner[j] = (int)i * 10 + j;
        }
        vector_push_back(vec, inner);
    }

    /*  Verify */
    size_t len;
    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 3)
        return 1;
    for (i = 0; i < len; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            if (vec[i][j] != (int)(i * 10 + j))
            {
                printf("Failed Vector of Vectors test.\n");
                return 1;
            }
        }
    }

    /*  Free inner vectors */
    for (i = 0; i < len; ++i)
    {
        free(vec[i]);
    }

    vector_free(vec);
    return 0;
}

int test_vector_of_vectors(void)
{
    Allocator a = {malloc, realloc, free};

    typedef int *IntVector;
    IntVector *vec = vector(IntVector, &a);
    if (!vec)
        return 1;

    /*  Create a few inner VECTORS manually */
    size_t i,j;
    for (i = 0; i < 3; ++i)
    {
        int *inner = vector(int, &a);
        if (!inner)
            return 1;

        for (j = 0; j < 4; ++j)
        {
            vector_push_back(inner, (int)i * 10 + j);
        }

        vector_push_back(vec, inner);
    }

    /*  Verify */
    size_t len;
    if (vector_get_len(vec, &len) != 0)
        return 1;
    if (len != 3)
        return 1;
    for (i = 0; i < len; ++i)
    {
        size_t inner_len;
        if (vector_get_len(vec[i], &inner_len) != 0)
            return 1;
        if (inner_len != 4)
            return 1;

        for (j = 0; j < inner_len; ++j)
        {
            if (vec[i][j] != (int)(i * 10 + j))
            {
                printf("Failed Vector of Vectors test at [%zu][%zu]\n", i, j);
                return 1;
            }
        }
    }

    /*  Free inner vectors */
    for (i = 0; i < len; ++i)
    {
        vector_free(vec[i]);
    }

    /*  Free outer vector */
    vector_free(vec);

    return 0;
}

/*  -------- Main Test Runner -------- */

int main(void)
{
    int total = 11;
    int score = 0;

    score += (test_init_free() == 0);
    score += (test_append0() == 0);
    score += (test_pop_back() == 0);
    score += (test_resize() == 0);
    score += (test_can_append() == 0);
    score += (test_get_cap() == 0);
    score += (test_set_len() == 0);
    score += (test_remove() == 0);
    score += (test_remove_ordered() == 0);
    score += (test_vector_of_dyn() == 0);
    score += (test_vector_of_vectors() == 0);

    printf("%d/%d tests passed.\n", score, total);
    return 0;
}
