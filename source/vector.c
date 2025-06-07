#include "vector.h"
#include <string.h>

typedef struct
{
    size_t cap;   /* total capacity */
    size_t len;   /* current length */
    size_t tsize; /* type size*/
    allocator_t *a; /* allocator pointer */
} vector_header_t;

typedef unsigned char byte_t;

#define VECTOR_HEADER(vector) ((vector_header_t *)((byte_t *)vector - sizeof(vector_header_t)))

/* Initialize a new vector */
void *vector_init(size_t tsize, size_t cap, allocator_t *a)
{
    if (!a)
    {
        VECTOR_DEBUG_PERROR("Vector Init: given null allocator.\n");
        return NULL;
    }
    vector_header_t h = {cap, 0, tsize, a};
    void *ret = a->malloc(sizeof(h) + tsize * cap);
    if (!ret)
    {
        VECTOR_DEBUG_PERROR("Vector Init: allocation failed.\n");
        return NULL;
    }
    vector_header_t *hdr = ret;
    *hdr = h;
    return (byte_t *)ret + sizeof(h);
}

/* Free a vector */
vector_status_t vector_free(void *vector)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Free: given null vector.\n");
        return VEC_ERR;
    }
    vector_header_t *hdr = VECTOR_HEADER(vector);
    if (!hdr->a)
    {
        VECTOR_DEBUG_PERROR("Vector Free: null allocator in header.\n");
        return VEC_ERR;
    }
    hdr->a->free(hdr);
    return VEC_OK;
}

/* Check if vector can append */
vector_status_t vector_can_append(void *vector)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Can Append: given null vector.\n");
        return VEC_ERR;
    }
    vector_header_t *hdr = VECTOR_HEADER(vector);
    return (hdr->cap > hdr->len) ? VEC_OK : VEC_FULL;
}

/* Get vector capacity */
vector_status_t vector_get_cap(void *vector, size_t *out)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Get Cap: given null vector.\n");
        return VEC_ERR;
    }
    if (!out)
    {
        VECTOR_DEBUG_PERROR("Vector Get Cap: given null output pointer.\n");
        return VEC_ERR;
    }
    vector_header_t *hdr = VECTOR_HEADER(vector);
    *out = hdr->cap;
    return VEC_OK;
}

/* Get vector length */
vector_status_t vector_get_len(void *vector, size_t *out)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Get Len: given null vector.\n");
        return VEC_ERR;
    }
    if (!out)
    {
        VECTOR_DEBUG_PERROR("Vector Get Len: given null output pointer.\n");
        return VEC_ERR;
    }
    vector_header_t *hdr = VECTOR_HEADER(vector);
    *out = hdr->len;
    return VEC_OK;
}

/* Unordered remove */
vector_status_t vector_remove(void *vector, size_t index)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Remove: given null vector.\n");
        return VEC_ERR;
    }
    vector_header_t *hdr = VECTOR_HEADER(vector);

    if (index >= hdr->len)
    {
        VECTOR_DEBUG_PERROR("Vector Remove: index out of bounds.\n");
        return VEC_INDEX_OOB;
    }

    if (hdr->len > 1 && index != hdr->len - 1)
    {
        /* Move entire tail of the array backward one space */
        memmove((byte_t *)vector + hdr->tsize * index,
                (byte_t *)vector + hdr->tsize * (index + 1),
                (hdr->len - index - 1) * hdr->tsize);
    }

    hdr->len--;
    return VEC_OK;
}

/* Respects order */
vector_status_t vector_remove_ordered(void *vector, size_t index)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Remove Ordered: given null vector.\n");
        return VEC_ERR;
    }

    vector_header_t *hdr = VECTOR_HEADER(vector);
    if (index >= hdr->len)
    {
        VECTOR_DEBUG_PERROR("Vector Remove Ordered: index out of bounds.\n");
        return VEC_INDEX_OOB;
    }

    if (hdr->len > 1 && index != hdr->len - 1)
    {
        memmove((byte_t *)vector + hdr->tsize * index,
                (byte_t *)vector + hdr->tsize * (index + 1),
                hdr->tsize);
    }

    hdr->len--;
    return VEC_OK;
}

void *vector_shrink_to_fit(void *vector)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Shrink to Fit: given null vector.\n");
        return NULL;
    }

    vector_header_t *hdr = VECTOR_HEADER(vector);

    void *new_vec = vector_resize(vector, hdr->len);
    if (!new_vec)
    {
        VECTOR_DEBUG_PERROR("Vector Shrink to Fit: resize failed.\n");
        return NULL;
    }

    return new_vec;
}

void *vector_normal_copy(void *vector, void *(*malloc_fn)(size_t))
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Normal Copy: given null vector.\n");
        return NULL;
    }
    if (!malloc_fn)
    {
        VECTOR_DEBUG_PERROR("Vector Normal Copy: given null malloc function.\n");
        return NULL;
    }

    vector_header_t *hdr = VECTOR_HEADER(vector);

    size_t total_size = hdr->len * hdr->tsize;
    if (total_size == 0)
        return NULL;

    void *raw = malloc_fn(total_size);
    if (!raw)
    {
        VECTOR_DEBUG_PERROR("Vector Normal Copy: malloc failed.\n");
        return NULL;
    }

    memcpy(raw, vector, total_size);
    return raw;
}

/* Resize vector capacity */
void *vector_resize(void *vector, size_t cap)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Resize: given null vector.\n");
        return NULL;
    }
    vector_header_t *hdr = VECTOR_HEADER(vector);
    if (!hdr->a)
    {
        VECTOR_DEBUG_PERROR("Vector Resize: null allocator in header.\n");
        return NULL;
    }
    vector_header_t *new_vector = hdr->a->realloc(hdr, cap * hdr->tsize + sizeof(vector_header_t));
    if (!new_vector)
    {
        VECTOR_DEBUG_PERROR("Vector Resize: realloc failed.\n");
        return NULL;
    }
    new_vector->cap = cap;
    if (new_vector->len > cap)
        new_vector->len = cap;
    return (byte_t *)new_vector + sizeof(vector_header_t);
}

/* Pop back: reduce length and return pointer to popped item */
vector_status_t vector_pop_back(void *vector, void *out)
{
    if (!vector || !out)
    {
        VECTOR_DEBUG_PERROR("Vector Pop Back: given null vector or out.\n");
        return VEC_ERR;
    }
    vector_header_t *hdr = VECTOR_HEADER(vector);
    if (hdr->len == 0)
    {
        VECTOR_DEBUG_PERROR("Vector Pop Back: empty vector.\n");
        return VEC_EMPTY;
    }
    hdr->len--;
    memcpy(out, (byte_t *)vector + (hdr->len * hdr->tsize), hdr->tsize);
    return VEC_OK;
}

const char *vector_status_to_string(vector_status_t status)
{
    switch (status)
    {
    case VEC_OK:
        return "VEC_OK";
    case VEC_ERR:
        return "VEC_ERR";
    case VEC_FULL:
        return "VEC_FULL";
    case VEC_EMPTY:
        return "VEC_EMPTY";
    case VEC_INDEX_OOB:
        return "VEC_INDEX_OOB";
    default:
        return "Unknown Vector Status";
    }
}

void *internal_vector_prepare_push_back(void *vptr, size_t item_size)
{
    if (!vptr)
    {
        VECTOR_DEBUG_PERROR("Vector Push Back: given null.\n");
        return NULL;
    }

    size_t len, cap;
    vector_get_cap(vptr, &cap);
    vector_get_len(vptr, &len);

    if (vector_can_append(vptr) != VEC_OK)
    {
        void *tmp = vector_resize(vptr, (cap + 1) * 2);
        if (!tmp)
        {
            VECTOR_DEBUG_PERROR("Vector Push Back: resize failed.\n");
            return NULL;
        }
        vptr = tmp;
    }

    return vptr;
}

void *internal_vector_prepare_insert(void *vptr, size_t item_size, size_t index)
{
    if (!vptr)
    {
        VECTOR_DEBUG_PERROR("Vector Insert: given null.\n");
        return NULL;
    }

    size_t len, cap;
    vector_get_cap(vptr, &cap);
    vector_get_len(vptr, &len);

    if (index > len)
    {
        VECTOR_DEBUG_PERROR("Vector Insert: index out of bounds.\n");
        return NULL;
    }

    if (vector_can_append(vptr) != VEC_OK)
    {
        void *tmp = vector_resize(vptr, (cap + 1) * 2);
        if (!tmp)
        {
            VECTOR_DEBUG_PERROR("Vector Insert: resize failed.\n");
            return NULL;
        }
        vptr = tmp;
    }

    /*shift elements right (leave space for new item) */
    memmove((char *)vptr + (index + 1) * item_size,
            (char *)vptr + index * item_size,
            (len - index) * item_size);

    return vptr;
}

/* Set vector length (no bounds check) */
void internal_vector_set_len(void *vector, size_t len)
{
    if (!vector)
    {
        VECTOR_DEBUG_PERROR("Vector Set Len: given null vector.\n");
        return;
    }
    vector_header_t *hdr = VECTOR_HEADER(vector);
    hdr->len = len;
}