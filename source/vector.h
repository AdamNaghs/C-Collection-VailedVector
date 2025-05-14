#ifndef _VECTOR_H
#define _VECTOR_H

#include <stddef.h>

#define VECTOR_DEFAULT_CAP 64

/**
 * @brief Allocator interface for the vector system.
 */
typedef struct Allocator
{
    void *(*malloc)(size_t);   /**< Function to allocate memory. */
    void *(*realloc)(void *, size_t); /**< Function to reallocate memory. */
    void (*free)(void *);       /**< Function to free memory. */
} Allocator;

#ifdef VECTOR_DEBUG
#include <stdio.h>
#define VECTOR_DEBUG_PERROR(string) perror(string)
#else
#define VECTOR_DEBUG_PERROR(string)
#endif

/**
 * @brief Create a new vector of type T using a specified allocator.
 *
 * @param T Type of the elements.
 * @param a Pointer to Allocator.
 * @return T* Pointer to the start of the vector's elements.
 */
#define vector(T, a) (T *)vector_init(sizeof(T), VECTOR_DEFAULT_CAP, a)

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
        if (!(v))                                                          \
        {                                                                  \
            VECTOR_DEBUG_PERROR("Vector Push Back: given null.\n");        \
            break;                                                         \
        }                                                                  \
        size_t _len, _cap;                                                 \
        vector_get_cap(v, &_cap);                                          \
        vector_get_len(v, &_len);                                          \
        if (!vector_can_append(v))                                         \
        {                                                                  \
            void *_tmp = vector_resize(v, (_cap + 1) * 2);                 \
            if (!_tmp)                                                     \
            {                                                              \
                VECTOR_DEBUG_PERROR("Vector Push Back: resize failed.\n"); \
                break;                                                     \
            }                                                              \
            (v) = _tmp;                                                    \
        }                                                                  \
        (v)[_len++] = (item);                                              \
        vector_set_len(v, _len);                                           \
    } while (0)

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
 * @return 0 on success, 1 on error.
 */
int vector_free(void *vector);

/**
 * @brief Pop the last element from the vector.
 *
 * @warning Do not keep the returned pointer alive for long; it may become invalid after further vector operations.
 *
 * @param vector Vector pointer.
 * @return Pointer to the popped element, or NULL if vector is empty or null.
 */
void *vector_pop_back(void *vector);

/**
 * @brief Check if a vector can append without resizing.
 *
 * @param vector Vector pointer.
 * @return 1 if can append, 0 otherwise.
 */
int vector_can_append(void *vector);

/**
 * @brief Get the current capacity of the vector.
 *
 * @param vector Vector pointer.
 * @param out Pointer to size_t where the capacity will be written.
 * @return 0 on success, 1 on error.
 */
int vector_get_cap(void *vector, size_t *out);

/**
 * @brief Get the current length (number of elements) of the vector.
 *
 * @param vector Vector pointer.
 * @param out Pointer to size_t where the length will be written.
 * @return 0 on success, 1 on error.
 */
int vector_get_len(void *vector, size_t *out);

/**
 * @brief Resize the vector to a new capacity.
 *
 * @param vector Vector pointer.
 * @param cap New capacity.
 * @return Pointer to resized vector on success, NULL on failure.
 */
void *vector_resize(void *vector, size_t cap);

/**
 * @brief Set the length of the vector.
 *
 * @param vector Vector pointer.
 * @param len New length.
 * @return 0 on success, 1 on error.
 */
int vector_set_len(void *vector, size_t len);

/**
 * @brief Remove index from vector. Doesn't respect order.
 *
 * @param vector Vector pointer.
 * @param index Index to be removed.
 * @return 0 on success, 1 on error.
 */
int vector_remove(void* vector, size_t index);

/**
 * @brief Remove index from vector. Respects order.
 *
 * @param vector Vector pointer.
 * @param index Index to be removed.
 * @return 0 on success, 1 on error.
 */
int vector_remove_ordered(void* vector, size_t index);

#endif /* _VECTOR_H */
