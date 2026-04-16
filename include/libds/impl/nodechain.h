/**
 * @file    nodechain.h
 * @brief   Low-level node chain management (unsafe for direct use).
 *
 * @warning THIS HEADER IS NOT A PART OF THE PUBLIC API.
 * These functions operate on raw bytes and provide NO TYPE-SAFETY.
 * It is intended for INTERNAL USE ONLY by higher-level type-safe
 * data structures. Direct use may lead to MEMORY CORRUPTION or
 * UNDEFINED BEHAVIOR.
 *
 * @author  Gabriel Souza
 * @date    2026-02-19
 */

#ifndef LIBDS_IMPL_NODECHAIN_H
#define LIBDS_IMPL_NODECHAIN_H

#include <stddef.h>
#include <stdbool.h>
#include "libds/core.h"

/**
 * @defgroup NodeChainInternals Singly-Linked Node Structures Internals
 * @brief    Raw memory node pool management (type‑unsafe).
 * @{
 */

//==============================================================================
// Life-cycle Management
//==============================================================================

/**
 * @brief       Allocates a new empty node chain.
 *
 * @param[in]   value_size   Size (in bytes) of each stored value.
 * @param[in]   value_align  Alignment requirement of the stored value.
 *
 * @return  Pointer to the new chain, or NULL if @p value_size / @p value_align
 * are invalid or on allocation failure.
 *
 * @details Computes the node stride and data offset based on memory alignment.
 * The chain manages a pool of fixed‑size nodes that can be recycled.
 */
struct ds_node_chain *
ds_nc_alloc(size_t value_size, size_t value_align);

/**
 * @brief   Frees the entire node chain and all its managed memory.
 *
 * @param[in,out] chain_ref Double pointer to the chain (set to NULL on success).
 * @param[in]     destroy   Optional destructor for active elements (may be NULL).
 *
 * @return  DS_ERR_NONE on success, or
 * DS_ERR_NULL_POINTER if pointer arguments are invalid.
 *
 * @note    The pointer is set to NULL on success.
 * @details If @p destroy is provided, it is invoked on every active node's data.
 * All allocated heap blocks are returned to the OS, and @p *chain_ref is nullified.
 *
 * @par Complexity
 * - Time:  O(N)
 * - Space: O(1)
 */
enum ds_error
ds_nc_free(struct ds_node_chain **chain_ref, ds_destructor_fn destroy);

/**
 * @brief   Removes all active nodes but retains the allocated memory pool.
 *
 * @param[in]  chain         Pointer to the chain.
 * @param[in]  destroy       Optional destructor for each active element (may be NULL).
 * @param[in]  is_deep_clear Flag to allow or not recycling nodes
 *                           (`true` to free, `false` to recycle).
 *
 * @return  DS_ERR_NONE on success, or
 * DS_ERR_NULL_POINTER if pointer arguments are invalid.
 *
 * @note If @p is_deep_clear == `false`, all active nodes are rapidly shifted to the
 * free stack for recycling, ensuring subsequent insertions require zero allocations.
 *
 * @par Complexity
 * - Time:  O(1)
 * - Space: O(1)
 */
enum ds_error
ds_nc_clear(struct ds_node_chain *chain, ds_destructor_fn destroy, bool is_deep_clear);

/**
 * @brief   Deep copies all nodes from a source chain to a destination chain.
 *
 * @param[in,out] dst_chain   Destination chain (must already be allocated).
 * @param[in]     src_chain   Source chain (read‑only).
 * @param[in]     value_size  Size of each value (used for memcpy if @p copy is NULL).
 * @param[in]     copy        Optional custom copy routine (if NULL, uses memcpy).
 * @param[in]     destroy     Optional destructor for original destination elements.
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_ALLOCATION_FAILED on memory exhaustion/fragmentation, or
 * DS_ERR_COPY_FAILED if the custom copy function fails.
 *
 * @note    On failure, the destination chain is safely rolled back to its original state.
 * @details Utilizes @p dst_chain->node_stack to bypass heap allocations whenever possible.
 *
 * @par Complexity
 * - Time:  O(N + M)
 * - Space: O(M) worst case during pool expansion
 * Where N is @p dst_chain->length and M is @p src_chain->length.
 */
enum ds_error
ds_nc_copy(struct ds_node_chain *dst_chain, const struct ds_node_chain *src_chain,
           size_t value_size, ds_copier_fn copy, ds_destructor_fn destroy);


//==============================================================================
// Utilities
//==============================================================================

/**
 * @brief   Reverses the order of nodes in the chain in-place.
 *
 * @param[in,out] chain  Pointer to the chain.
 *
 * @return  DS_ERR_NONE on success, or
 * DS_ERR_NULL_POINTER if pointer arguments are invalid.
 *
 * @par Complexity
 * - Time:  O(N)
 * - Space: O(1)
 */
enum ds_error
ds_nc_reverse(struct ds_node_chain *chain);

/**
 * @brief   Returns the number of active nodes currently holding data.
 *
 * @param[in]  chain  Pointer to the chain.
 *
 * @return  Number of active nodes, or 0 if chain is NULL.
 *
 * @par Complexity
 * - Time:  O(1)
 * - Space: O(1)
 */
size_t
ds_nc_length(const struct ds_node_chain *chain);

/**
 * @brief   Calculates the total heap memory footprint of the chain.
 *
 * @param[in]  chain  Pointer to the chain.
 *
 * @return  Total bytes allocated on the heap, or 0 if chain is NULL.
 *
 * @par Complexity
 * - Time:  O(log N), traverses the logarithmically scaled header chunks
 * - Space: O(1)
 */
size_t
ds_nc_bytes(const struct ds_node_chain *chain);

/**
 * @brief   Checks whether the chain is empty.
 *
 * @param[in]  chain  Pointer to the chain.
 *
 * @return  true if @p chain is NULL or has zero active nodes, false otherwise.
 *
 * @par Complexity
 * - Time:  O(1)
 * - Space: O(1)
 */
bool
ds_nc_is_empty(const struct ds_node_chain *chain);


//==============================================================================
// Get Value
//==============================================================================

/**
 * @brief   Retrieves a pointer to the data payload of the first node.
 *
 * @param[in]   chain  Pointer to the chain.
 * @param[out]  out    Pointer updated to point at the target data segment.
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid, or
 * DS_ERR_EMPTY_STRUCTURE if chain has no elements.
 *
 * @par Complexity
 * - Time:  O(1)
 * - Space: O(1)
 */
enum ds_error
ds_nc_get_front(const struct ds_node_chain *chain, void **out);

/**
 * @brief   Retrieves a pointer to the data payload of the last node.
 *
 * @param[in]   chain  Pointer to the chain.
 * @param[out]  out    Pointer updated to point at the target data segment.
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid, or
 * DS_ERR_EMPTY_STRUCTURE if chain has no elements.
 *
 * @par Complexity
 * - Time:  O(1)
 * - Space: O(1)
 */
enum ds_error
ds_nc_get_back(const struct ds_node_chain *chain, void **out);

/**
 * @brief   Retrieves a pointer to the data payload of the node at a given index.
 *
 * @param[in]   chain  Pointer to the chain.
 * @param[in]   index  Zero‑based position. Range: [0, length -1].
 * @param[out]  out    Pointer updated to point at the target data segment.
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid,
 * DS_ERR_EMPTY_STRUCTURE if chain has no elements, or
 * DS_ERR_INDEX_OUT_OF_BOUNDS if index is invalid.
 *
 * @par Complexity
 * - Time:  O(N) linked list traversal
 * - Space: O(1)
 */
enum ds_error
ds_nc_get_at(const struct ds_node_chain *chain, size_t index, void **out);


//==============================================================================
// Push Value
//==============================================================================

/**
 * @brief   Allocates and inserts a new node at the front of the chain.
 *
 * @param[in,out] chain  Pointer to the chain.
 * @param[out]    out    Pointer updated to the newly allocated data payload area.
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid, or
 * DS_ERR_ALLOCATION_FAILED on memory exhaustion/fragmentation.
 *
 * @details The returned @p out pointer must be used to write the value directly.
 * The node is already securely linked into the chain's active state.
 *
 * @par Complexity
 * - Time:  O(1) amortized
 * - Space: O(1) amortized
 * Worst case during pool growth
 */
enum ds_error
ds_nc_push_front(struct ds_node_chain *chain, void **out);

/**
 * @brief   Allocates and appends a new node at the back of the chain.
 *
 * @param[in,out] chain  Pointer to the chain.
 * @param[out]    out    Pointer updated to the newly allocated data payload area.
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid, or
 * DS_ERR_ALLOCATION_FAILED on memory exhaustion/fragmentation.
 *
 * @par Complexity
 * - Time:  O(1) amortized
 * - Space: O(1) amortized
 * Worst case during pool growth
 */
enum ds_error
ds_nc_push_back(struct ds_node_chain *chain, void **out);

/**
 * @brief   Allocates and inserts a new node at the specified index.
 *
 * @param[in,out] chain  Pointer to the chain.
 * @param[in]     index  Zero‑based position. Range: [0, length].
 * @param[out]    out    Pointer updated to the newly allocated data payload area.
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid,
 * DS_ERR_ALLOCATION_FAILED on memory exhaustion/fragmentation, or
 * DS_ERR_INDEX_OUT_OF_BOUNDS if index is invalid.
 *
 * @details If @p index == length, it acts as push_back.
 *
 * @par Complexity
 * - Time:  O(N) (O(1) if index is 0 or equals length)
 * - Space: O(1) amortized
 * Worst case during pool growth
 */
enum ds_error
ds_nc_push_at(struct ds_node_chain *chain, size_t index, void **out);


//==============================================================================
// Pop Value
//==============================================================================

/**
 * @brief   Removes the first node and returns it to the recycling pool.
 *
 * @param[in,out] chain    Pointer to the chain.
 * @param[out]    out      Optional pointer to view data before removal (may be NULL).
 * @param[in]     destroy  Optional destructor for the removed element (may be NULL).
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid, or
 * DS_ERR_EMPTY_STRUCTURE if chain has no elements.
 *
 * @details The node memory is strictly recycled internally, not freed to the OS.
 *
 * @par Complexity
 * - Time:  O(1)
 * - Space: O(1)
 */
enum ds_error
ds_nc_pop_front(struct ds_node_chain *chain, void **out, ds_destructor_fn destroy);

/**
 * @brief   Removes the last node and returns it to the recycling pool.
 *
 * @param[in,out]   chain   Pointer to the chain.
 * @param[out]      out     Optional output pointer to view data before destruction (may be NULL).
 * @param[in]       destroy Optional destructor for the removed element (may be NULL).
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid, or
 * DS_ERR_EMPTY_STRUCTURE if chain has no elements.
 *
 * @par Complexity
 * - Time:  O(N), traversal required to update the new tail
 * - Space: O(1)
 */
enum ds_error
ds_nc_pop_back(struct ds_node_chain *chain, void **out, ds_destructor_fn destroy);

/**
 * @brief   Removes the node at the specified index and returns it to the recycling pool.
 *
 * @param[in,out]   chain   Pointer to the chain.
 * @param[in]       index   Zero‑based position. Range: [0, length -1].
 * @param[out]      out     Optional output pointer to view data before destruction (may be NULL).
 * @param[in]       destroy Optional destructor for the removed element (may be NULL).
 *
 * @return  DS_ERR_NONE on success,
 * DS_ERR_NULL_POINTER if pointer arguments are invalid,
 * DS_ERR_EMPTY_STRUCTURE if chain has no elements, or
 * DS_ERR_INDEX_OUT_OF_BOUNDS if index is invalid.
 *
 * @par Complexity
 * - Time:  O(N)
 * - Space: O(1)
 */
enum ds_error
ds_nc_pop_at(struct ds_node_chain *chain, size_t index, void **out, ds_destructor_fn destroy);

/** @} */ //end of NodeChainInternals group

#endif //LIBDS_IMPL_NODECHAIN_H