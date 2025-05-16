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

/**
 * @brief Push an item onto the end of the vector.
 *
 * Automatically resizes if necessary. Debug prints on errors if VECTOR_DEBUG is enabled.
 *
 * @param v Vector pointer.
 * @param item Item to push.
 */

#define vector_push_back(v, item) internal_vector_push_back(v, item)

/**
 * @brief
 *
 * @param v Vector pointer.
 * @param source Source array, can be value type array i.e. {1, 2, 3}.
 * @param len Length of source array.
 */
#define vector_push_many(v, source, len) internal_vector_push_many(v, source, len)

/**
 * @brief Push an item onto the end of the vector.
 *
 * Automatically resizes if necessary. Debug prints on errors if VECTOR_DEBUG is enabled.
 *
 * @param v Vector pointer.
 * @param index Index to insert item to.
 * @param item Item to push.
 */
#define vector_insert(v, index, item) internal_vector_insert(v, index, item)

/**
 * @brief Shrinks the capacity of the vector to fit its current length.
 *
 * This releases any unused memory beyond the current number of elements.
 *
 * @param v Vector pointer (will be reassigned).
 * @return The shrunk vector pointer (same as v), or NULL on failure.
 */
#define vector_shrink(v) ((v) = vector_shrink_to_fit(v))

/**
 * @brief Iterate over a vector with automatic type deduction.
 *
 * @param T The type of each element.
 * @param v The vector pointer.
 * @param var A variable of type T to receive each element during iteration.
 *
 * Example:
 * @code
 * int *vec = vector(int, &a);
 * vector_push_back(vec, 10);
 * int val = 0;
 * vector_foreach(vec, val) {
 *     printf("%d\n", val);
 * }
 * @endcode
 */
#define vector_foreach(v, var)                                                                                      \
    for (size_t _i = 0, _len = 0;                                                                                   \
         (v) && ((_i < (_len == 0 && vector_get_len((v), &_len) == VEC_OK ? _len : _len)) && ((var) = (v)[_i], 1)); \
         ++_i)

/**
 * @brief ANSI-compatible foreach that exposes index and length.
 *
 * Use this when you need both the index and element. Also avoids shadowing.
 *
 * @param _i The loop index variable.
 * @param _len A variable to hold the total vector length.
 * @param v The vector pointer.
 * @param var A variable to receive each element.
 *
 * Example:
 * @code
 * size_t i, len;
 * int item;
 * vector_foreach_ansi(i, len, vec, item) {
 *     printf("[%zu] %d\n", i, item);
 * }
 * @endcode
 */
#define vector_foreach_ansi(_i, _len, v, var)                                                                       \
    for (_i = 0, _len = 0;                                                                                          \
         (v) && ((_i < (_len == 0 && vector_get_len((v), &_len) == VEC_OK ? _len : _len)) && ((var) = (v)[_i], 1)); \
         ++_i)

/* Internal methdods */

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

void *internal_vector_prepare_push_back(void *vptr, size_t item_size);

void *internal_vector_prepare_insert(void *vptr, size_t item_size, size_t index);

void internal_vector_set_len(void *vector, size_t len);

#define internal_vector_push_back(v, item)                                 \
    do                                                                     \
    {                                                                      \
        void *_tmp = internal_vector_prepare_push_back((v), sizeof(*(v))); \
        if (!_tmp)                                                         \
            break;                                                         \
        (v) = _tmp; /* Resize if needed */                                 \
        size_t _len;                                                       \
        vector_get_len(v, &_len);                                          \
        (v)[_len++] = (item);                                              \
        internal_vector_set_len(v, _len);                                  \
    } while (0)

#define internal_vector_push_many(v, source, len)                   \
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

#define internal_vector_insert(v, index, item)                                   \
    do                                                                           \
    {                                                                            \
        void *_tmp = internal_vector_prepare_insert((v), sizeof(*(v)), (index)); \
        if (!_tmp)                                                               \
            break;                                                               \
        (v) = _tmp;                                                              \
        (v)[(index)] = (item);                                                   \
        size_t _len;                                                             \
        vector_get_len(v, &_len);                                                \
        internal_vector_set_len(v, _len + 1);                                    \
    } while (0)

#endif /* _VECTOR_H */
