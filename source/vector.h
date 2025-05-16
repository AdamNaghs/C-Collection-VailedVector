#ifndef _VECTOR_H
#define _VECTOR_H

#include <stddef.h>
#include <stdint.h>

#define VECTOR_DEFAULT_CAP 16

typedef enum
{
    VEC_OK = 0,
    VEC_ERR,
    VEC_FULL,
    VEC_EMPTY,
    VEC_INDEX_OOB
} VectorStatus;

/**
 * @brief Allocator interface for the vector system.
 */
typedef struct Allocator
{
    void *(*malloc)(size_t);          /**< Function to allocate memory. */
    void *(*realloc)(void *, size_t); /**< Function to reallocate memory. */
    void (*free)(void *);             /**< Function to free memory. */
} Allocator;

/**
 * @brief Create a new vector of type T using a specified allocator.
 *
 * @param T Type of the elements.
 * @param a Pointer to Allocator.
 * @return T* Pointer to the start of the vector's elements.
 */
#define vector(T, a) (T *)vector_init(sizeof(T), VECTOR_DEFAULT_CAP, a)

/**
 * @brief Initialize a vector.
 *
 * @param tsize Size of each element (sizeof(T)).
 * @param cap Initial capacity.
 * @param a Pointer to Allocator.
 * @return void* Pointer to elements on success, NULL on failure.
 */
void *vector_init(size_t tsize, size_t cap, Allocator *a);

/**
 * @brief Free a vector.
 *
 * @param vector Vector pointer.
 * @return VEC_OK on success, VEC_ERR on error
 */
VectorStatus vector_free(void *vector);

/**
 * @brief Check if a vector can append without resizing.
 *
 * @param vector Vector pointer.
 * @return VEC_OK if vector can append, VEC_FULL if full, VEC_ERR on error.
 */
VectorStatus vector_can_append(void *vector);

/**
 * @brief Get the current capacity of the vector.
 *
 * @param vector Vector pointer.
 * @param out Pointer to size_t where the capacity will be written.
 * @return VEC_OK on success, VEC_ERR on error
 */
VectorStatus vector_get_cap(void *vector, size_t *out);

/**
 * @brief Get the current length (number of elements) of the vector.
 *
 * @param vector Vector pointer.
 * @param out Pointer to size_t where the length will be written.
 * @return VEC_OK on success, VEC_ERR on error
 */
VectorStatus vector_get_len(void *vector, size_t *out);

/**
 * @brief Set the length of the vector. Doesn't do bounds checking. Feel free to ignore, its just used by a macro.
 *
 * @param vector Vector pointer.
 * @param len New length.
 * @return VEC_OK on success, VEC_ERR on error
 */
VectorStatus vector_set_len(void *vector, size_t len);

/**
 * @brief Remove index from vector. Doesn't respect order.
 *
 * @param vector Vector pointer.
 * @param index Index to be removed.
 * @return VEC_OK on success, VEC_INDEX_OOB if index is out of bounds, VEC_ERR on error
 */
VectorStatus vector_remove(void *vector, size_t index);

/**
 * @brief Remove index from vector. Respects order.
 *
 * @param vector Vector pointer.
 * @param index Index to be removed.
 * @return VEC_OK on success, VEC_INDEX_OOB if index is out of bounds, VEC_ERR on error
 */
VectorStatus vector_remove_ordered(void *vector, size_t index);

/**
 * @brief Copies removes last value from vector and copies it to out.
 *
 * @param vector Vector pointer
 * @param out Reference to copy pop value to.
 */
VectorStatus vector_pop_back(void *vector, void *out);

/**
 * @brief Copies the vector contents into a normal C array (no header).
 *
 * @param vector Vector pointer.
 * @param malloc_fn malloc-like function for allocating the array.
 * @return Pointer to plain array or NULL on failure.
 */
void *vector_normal_copy(void *vector, void *(*malloc_fn)(size_t));

/**
 * @brief Resize the vector to a new capacity.
 *
 * @param vector Vector pointer.
 * @param cap New capacity.
 * @return Pointer to resized vector on success, NULL on failure.
 */
void *vector_resize(void *vector, size_t cap);

/**
 * @brief Resizes capacity to match length;
 *
 * @param vector Reference to Vector pointer.
 * @return VEC_OK on success, VEC_ERR on error
 */
void *vector_shrink_to_fit(void *vector_ptr);

/**
 * @brief Returns string that matches status
 *
 * @param status Vector function return code
 * @return const char*
 */
const char *vector_status_to_string(VectorStatus status);

#ifdef VECTOR_DEBUG
#include <stdio.h>
#define VECTOR_DEBUG_PERROR(string) perror(string)
#define VECTOR_VALIDATE(v)                                                          \
    do                                                                              \
    {                                                                               \
        if (((uintptr_t)(v)) % sizeof(void *) != 0)                                 \
        {                                                                           \
            VECTOR_DEBUG_PERROR("Vector Validate: vector not properly aligned.\n"); \
        }                                                                           \
    } while (0)
#else
#define VECTOR_DEBUG_PERROR(string)
#define VECTOR_VALIDATE(v)
#endif

/**
 * @brief Push an item onto the end of the vector.
 *
 * Automatically resizes if necessary. Debug prints on errors if VECTOR_DEBUG is enabled.
 *
 * @param v Vector pointer.
 * @param item Item to push.
 */
#define vector_push_back(v, item)                                          \
    do                                                                     \
    {                                                                      \
        void *_tmp = internal_vector_prepare_push_back((v), sizeof(*(v))); \
        if (!_tmp)                                                         \
            break;                                                         \
        (v) = _tmp; /* Resize if needed */                                 \
        size_t _len;                                                       \
        vector_get_len(v, &_len);                                          \
        (v)[_len++] = (item);                                              \
        vector_set_len(v, _len);                                           \
    } while (0)

/**
 * @brief
 *
 * @param v Vector pointer.
 * @param source Source array, can be value type array i.e. {1, 2, 3}.
 * @param len Length of source array.
 */
#define vector_push_many(v, source, len)                            \
    do                                                              \
    {                                                               \
        if (!(v))                                                   \
        {                                                           \
            VECTOR_DEBUG_PERROR("Vector Push Many: given null.\n"); \
            break;                                                  \
        }                                                           \
        size_t _vi;                                                 \
        for (_vi = 0; _vi < (size_t)(len); _vi++)                   \
        {                                                           \
            vector_push_back((v), (source)[_vi]);                   \
        }                                                           \
    } while (0)

/**
 * @brief Push an item onto the end of the vector.
 *
 * Automatically resizes if necessary. Debug prints on errors if VECTOR_DEBUG is enabled.
 *
 * @param v Vector pointer.
 * @param index Index to insert item to.
 * @param item Item to push.
 */
#define vector_insert(v, index, item)                                               \
    do                                                                              \
    {                                                                               \
        if (!(v))                                                                   \
        {                                                                           \
            VECTOR_DEBUG_PERROR("Vector Insert: given null.\n");                    \
            break;                                                                  \
        }                                                                           \
        size_t _len, _cap;                                                          \
        vector_get_cap(v, &_cap);                                                   \
        vector_get_len(v, &_len);                                                   \
        if ((index) > _len)                                                         \
        {                                                                           \
            VECTOR_DEBUG_PERROR("Vector Insert: index out of bounds.\n");           \
            break;                                                                  \
        }                                                                           \
        if (vector_can_append(v) != VEC_OK)                                         \
        {                                                                           \
            void *_tmp = vector_resize(v, (_cap + 1) * 2);                          \
            if (!_tmp)                                                              \
            {                                                                       \
                VECTOR_DEBUG_PERROR("Vector Insert: resize failed.\n");             \
                break;                                                              \
            }                                                                       \
            (v) = _tmp;                                                             \
        }                                                                           \
        memmove(&(v)[(index) + 1], &(v)[(index)], (_len - (index)) * sizeof(*(v))); \
        (v)[(index)] = (item);                                                      \
        vector_set_len(v, _len + 1);                                                \
    } while (0)

#define vector_shrink(v) ((v) = vector_shrink_to_fit(v))

#define vector_foreach(T, v, var)                                                                                   \
    for (size_t _i = 0, _len = 0;                                                                                   \
         (v) && ((_i < (_len == 0 && vector_get_len((v), &_len) == VEC_OK ? _len : _len)) && ((var) = (v)[_i], 1)); \
         ++_i)

#define vector_foreach_ansi(_i, _len, v, var)                                                                       \
    for (_i = 0, _len = 0;                                                                                          \
         (v) && ((_i < (_len == 0 && vector_get_len((v), &_len) == VEC_OK ? _len : _len)) && ((var) = (v)[_i], 1)); \
         ++_i)

/* Internal methdods */
void *internal_vector_prepare_push_back(void *vptr, size_t item_size);

#endif /* _VECTOR_H */
