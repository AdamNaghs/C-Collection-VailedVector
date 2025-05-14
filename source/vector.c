#include "vector.h"
#include <string.h>

typedef struct
{
    size_t cap;   /* total capacity */
    size_t len;   /* current length */
    size_t tsize; /* type size*/
    Allocator *a; /* allocator pointer */
} VectorHeader;

typedef unsigned char byte_t;

#define VECTOR_HEADER(vector) ((VectorHeader *)((byte_t *)vector - sizeof(VectorHeader)))

/* Initialize a new vector */
void *vector_init(size_t tsize, size_t cap, Allocator *a)
{
    if (!a)
    {
        VECTOR_DEBUG_PERROR("Vector Init: given null allocator.\n");
        return NULL;
    }
    VectorHeader h = {cap, 0, tsize, a};
    void *ret = a->malloc(sizeof(h) + tsize * cap);
    if (!ret)
    {
        VECTOR_DEBUG_PERROR("Vector Init: allocation failed.\n");
        return NULL;
    }
    VectorHeader *tmp = ret;
    *tmp = h;
    return (byte_t *)ret + sizeof(h);
}

/* Free a vector */
int vector_free(void *vector)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Free: given null vector.\n");
        return 1;
    }
    VectorHeader *tmp = VECTOR_HEADER(vector);
    if (!tmp->a)
    {
        VECTOR_DEBUG_PERROR("Vector Free: null allocator in header.\n");
        return 1;
    }
    tmp->a->free(tmp);
    return 0;
}

/* Check if vector can append */
int vector_can_append(void *vector)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Can Append: given null vector.\n");
        return 0;
    }
    VectorHeader *tmp = VECTOR_HEADER(vector);
    return tmp->cap > tmp->len;
}

/* Get vector capacity */
int vector_get_cap(void *vector, size_t *out)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Get Cap: given null vector.\n");
        return 1;
    }
    if (!out)
    {
        VECTOR_DEBUG_PERROR("Vector Get Cap: given null output pointer.\n");
        return 1;
    }
    VectorHeader *tmp = VECTOR_HEADER(vector);
    *out = tmp->cap;
    return 0;
}

/* Get vector length */
int vector_get_len(void *vector, size_t *out)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Get Len: given null vector.\n");
        return 1;
    }
    if (!out)
    {
        VECTOR_DEBUG_PERROR("Vector Get Len: given null output pointer.\n");
        return 1;
    }
    VectorHeader *tmp = VECTOR_HEADER(vector);
    *out = tmp->len;
    return 0;
}

/* Set vector length (no bounds check) */
int vector_set_len(void *vector, size_t len)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Set Len: given null vector.\n");
        return 1;
    }
    VectorHeader *tmp = VECTOR_HEADER(vector);
    tmp->len = len;
    return 0;
}

/* Resize vector capacity */
void *vector_resize(void *vector, size_t cap)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Resize: given null vector.\n");
        return NULL;
    }
    VectorHeader *tmp = VECTOR_HEADER(vector);
    if (!tmp->a)
    {
        VECTOR_DEBUG_PERROR("Vector Resize: null allocator in header.\n");
        return NULL;
    }
    VectorHeader *new_vector = tmp->a->realloc(tmp, cap * tmp->tsize + sizeof(VectorHeader));
    if (!new_vector)
    {
        VECTOR_DEBUG_PERROR("Vector Resize: realloc failed.\n");
        return NULL;
    }
    new_vector->cap = cap;
    return (byte_t *)new_vector + sizeof(VectorHeader);
}

/* Pop back: reduce length and return pointer to popped item */
void *vector_pop_back(void *vector)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Pop Back: given null vector.\n");
        return NULL;
    }
    VectorHeader *hdr = VECTOR_HEADER(vector);
    if (hdr->len == 0)
    {
        VECTOR_DEBUG_PERROR("Vector Pop Back: empty vector.\n");
        return NULL;
    }
    hdr->len--;
    return (byte_t *)vector + (hdr->len * hdr->tsize);
}

int vector_remove(void *vector, size_t index)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Remove: given null vector.\n");
        return 1;
    }
    VectorHeader *hdr = VECTOR_HEADER(vector);

    if (index >= hdr->len)
    {
        VECTOR_DEBUG_PERROR("Vector Remove: index out of bounds.\n");
        return 1;
    }

    if (hdr->len > 1 && index != hdr->len - 1)
    {
        /* Move entire tail of the array forward one space */
        memmove((byte_t *)vector + hdr->tsize * index,
                (byte_t *)vector + hdr->tsize * (index + 1),
                (hdr->len - index - 1) * hdr->tsize);
    }

    hdr->len--;
    return 0;
}

int vector_remove_ordered(void *vector, size_t index)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Remove Ordered: given null vector.\n");
        return 1;
    }

    VectorHeader *hdr = VECTOR_HEADER(vector);
    if (index >= hdr->len)
    {
        VECTOR_DEBUG_PERROR("Vector Remove Ordered: index out of bounds.\n");
        return 1;
    }

    if (hdr->len > 1 && index != hdr->len - 1)
    {
        memmove((byte_t *)vector + hdr->tsize * index,
                (byte_t *)vector + hdr->tsize * (index + 1),
                hdr->tsize);
    }

    hdr->len--;
    return 0;
}

void* vector_shrink_to_fit(void *vector)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Shrink to Fit: given null vector.\n");
        return NULL;
    }

    VectorHeader *hdr = VECTOR_HEADER(vector);

    void* new_vec = vector_resize(vector, hdr->len);
    if (!new_vec)
    {
        VECTOR_DEBUG_PERROR("Vector Shrink to Fit: resize failed.\n");
        return NULL;
    }

    return new_vec;
}
