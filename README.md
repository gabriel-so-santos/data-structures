# LibDS – A Data Structures & Algorithms Library in C

### A collection of data structures and algorithms implemented in C for learning and practice.

This library provides generic implementations of singly-linked lists, stacks, and queues through macro-generated wrapper
functions. It utilizes an intrusive memory layout to minimize allocations and provides a highly configurable geometric
memory pool.

#### Note: It requires C11 at minimum

#### Warning: The current implementation does NOT provide thread-safety.

---

## Features & Generation

Data structures are generated using type-safe macros. This prevents `void *` casting issues while maintaining a single,
unified backend implementation.


```c++
#include <libds/listdef.h>      // list  generator
#include <libds/stackdef.h>     // stack generator
#include <libds/queuedef.h>     // queue generator

LIBDS_DEF_LIST(Type, ListType, Prefix, CopyFunc, DestroyFunc)

LIBDS_DEF_STACK(Type, StackType, Prefix, CopyFunc, DestroyFunc)

LIBDS_DEF_QUEUE(Type, QueueType, Prefix, CopyFunc, DestroyFunc)
```


### Macro Parameters

| Parameter     | Description                                                                                                                    |
|---------------|--------------------------------------------------------------------------------------------------------------------------------|
| `Type`        | The data type to store (e.g., `int`, `char *`, `struct user`).                                                                 |
| `Type Name`   | The generated struct name for your container (e.g., `ListInt`).                                                                |
| `Prefix`      | The prefix applied to all generated functions (e.g., `li`).                                                                    |
| `CopyFunc`    | Pointer to a custom `ds_copier_fn`. Return `true` on success, `false` on failure. Pass `NULL` for simple assignment.           |
| `DestroyFunc` | Pointer to a custom `ds_destructor_fn` to free dynamically allocated payloads. Pass `NULL` if no internal freeing is required. |


### Example: Storing Primitives
```c++
#include <libds/stackdef.h>

LIBDS_DEF_STACK(double, StackDouble, stack_double, NULL, NULL)

int main()
{
    StackDouble numbers = stack_double_create();
    
    stack_double_push(numbers, 3.1415926535);
    
    double value;
    stack_double_pop(numbers, &value);
    
    stack_double_delete(&numbers);
    return 0;
}
```

### Example: Storing structs
```c++
#include <libds/queuedef.h>

// custom struct
struct user
{
    long long id;
    unsigned int age;
    char email[64];
    char username[64];
};

LIBDS_DEF_QUEUE(struct user, QueueUser, queue_user, NULL, NULL)

int main()
{
    QueueUser users = queue_user_create();
    
    struct user user_data = {
        .id = 1000012,
        .age = 25,
        .email = "user@example.com",
        .username = "User123"
    };

    queue_user_enqueue(users, user_data);
    
    struct user out;
    queue_user_dequeue(users, &out);
    
    queue_user_delete(&users);
    return 0;
}
```

### Example: Storing Pointers (Custom `copy` & `destroy`)
```c++
#include <stdlib.h>
#include <string.h>
#include <libds/listdef.h>

// setting up copy & destroy

bool copy_str(void *dst, const void *src)
{
    if (!dst || !src) return false;
    const size_t len = strlen(*(const char **)src);

    char *new_str = malloc(len + 1);
    if (!new_str) return false;
    new_str[len] = '\0';

    strncpy(new_str, *(const char **)src, len);
    *(char **)dst = new_str;
    return true;
}

void destroy_str(void *data)
{
    if (!data) return;
    free(*(char **)data);
    *(char **)data = NULL;
}

LIBDS_DEF_LIST(char *, ListStr, list_str, copy_str, destroy_str)

int main()
{
    ListStr rock_bands = list_str_create();
    
    list_str_prepend(rock_bands, "The Beatles"); // or list_str_push_front()
    list_str_append(rock_bands, "Queen");        // or list_str_push_back()
    list_str_push_at(rock_bands, 1, "AC/DC");
    
    /*
     * New order:
     * [ "Queen", "AC/DC", "The Beatles"]
     */
    list_str_reverse(rock_bands);
    
    
    list_str_drop_at(rock_bands, 2); // remove "The Beatles"
    
    char *out;
    list_str_pop_at(rock_bands, 0, &out); // pop "Queen"
    
    free(out); // ownership transferred to 'out'
    
    list_str_delete(&rock_bands); // automatically frees the remaining memory 
    return 0;
}
```

## Memory Ownership & Custom Copiers / Destructors

The memory of the `Node` itself is managed by the library, but the payload can be managed by `copy` and `destroy` functions.
Both functions receive a pointer to the payload directly:

```   
       Node          
+------------------+
| Offset           |
+------------------+
| Payload          |  <--- void *data
+------------------+
| Padding          |
+------------------+

```

### 1. Copy Function (`ds_copier_fn`)

The `copy` function is triggered whenever a new element is inserted into the container. 
Its signature is:

`bool custom_copy(void *dst, const void *src);`

- `src` (Source): A pointer to the valid data the user passed into the insertion function.
- `dst` (Destination): A pointer to raw, uninitialized memory inside the newly allocated node.
- Returns `true` on success, `false` on failure.

Because `dst` is uninitialized, it must fully construct the data there.
If the data type requires multiple heap allocations (e.g., a struct with two distinct string pointers),
and one of those allocations fails halfway through, the copy function must clean up the successful allocations before
returning `false`. Returning `false` tells the library
to abort the insertion and recycle the node.
If the function leave partially allocated memory behind, it will leak.


### 2. Destroy Function (`ds_destructor_fn`) & Ownership

The `destroy` function is triggered whenever an element is removed from the container. Its signature is:

`void custom_destroy(void *data);`

- `data`: A pointer to the payload inside the node about to be destroyed. It must free any dynamically allocated fields
inside the data, but must not free the data pointer itself (the library handles the node's memory).

### Automatic Destruction
The library automatically calls the destructor when invoking functions that discard data, such as:

- `delete()` and `clear()`

- `drop_front()`, `drop_back()`, `drop_at()`


### The Ownership Transfer Exception (pop and dequeue)
There is a critical exception to automatic destruction: **Ownership Transfer**.

When calling a function like `pop_at(list, index, &out)`, `pop(stack, &out)` or `dequeue(queue, &out)` and
providing a valid `out` pointer, they are asking the library to give the data back to them. In this scenario:
- The container copies the payload into the out pointer.
- The container skips the destructor.
- The node is recycled.
- The user now owns that memory. It is entirely the user's responsibility to `free()` it when they are done.

_Note: If the user passes `NULL` for the `out` pointer, the library assumes they do not want the data and will execute the destructor normally._

### Example: Copying Buffers (No dynamic allocation)

```c++
#include <string.h>
#include <libds/listdef>

typedef struct
{
    char buffer[16];
} Str16;

bool copy_str16(void *dst, const void *src)
{
    if (!dst || !src) return false;

    Str16 *d = (Str16 *)dst;
    const Str16 *s = (const Str16 *)src;
    *d = *s;
    
    const size_t len = strnlen(d->buffer, 15);
    d->buffer[len] = '\0'; // Guarantee null termination
    
    return true;
}

// No destructor needed since memory is statically sized inside the struct
LIBDS_DEF_LIST(Str16, ListStr16, list_str16, copy_str16, NULL);
```


### Example: Copying Pointers (Dynamic allocation)

Because the structure stores pointers, `src` and `dst` will be pointers to pointers `(char **)`.
```c++
#include <stdlib.h>
#include <string.h>
#include <libds/listdef.h>

bool copy_str(void *dst, const void *src)
{
    if (!dst || !src) return false;
    const size_t len = strlen(*(const char **)src);

    char *new_str = malloc(len + 1);
    if (!new_str) return false;
    new_str[len] = '\0';

    strncpy(new_str, *(const char **)src, len);
    *(char **)dst = new_str;
    return true;
}

void destroy_str(void *data)
{
    if (!data) return;
    free(*(char **)data);
    *(char **)data = NULL;
}

LIBDS_DEF_LIST(char *, ListStr, list_str, copy_str, destroy_str)
```

---


## API Reference

*Note: All functions will include `Prefix_` preceding their signature (e.g., `li_push_back`).*

### Common Functions (All Containers)

| Function                      | Time Complexity | Description                                                                                                                |
|:------------------------------|:----------------|:---------------------------------------------------------------------------------------------------------------------------|
| `create(void)`                | $O(1)$          | Allocates and initializes a new container.                                                                                 |
| `delete(&cont)`               | $O(N)$          | Frees all nodes, applies destructors, and nullifies the reference.                                                         |
| `clear(cont)`                 | $O(1)$*         | Moves all active nodes to the recycle stack. * $O(N)$ if a destructor is set).                                            |
| `deep_clear(cont)`            | $O(\log N)$*    | Clears data and frees recycled nodes. * $O(N)$ if a destructor is set; up to $O(N^2)$ when paired with linear allocation. |
| `copy(dst, src)`              | $O(N)$          | Performs a deep copy using the container's copier function. Rolls back safely on failure.                                  |
| `get_front(cont,⠀&out)`       | $O(1)$          | Retrieves the first element without removing it.                                                                           |
| `get_back(cont,⠀&out)`        | $O(1)$          | Retrieves the last element without removing it.                                                                            |
| `get_at(cont,⠀index,⠀&out)`   | $O(N)$          | Retrieves the element at the specified index within the range $[0, N)$.                                                    |
| `length(cont)` / `size(cont)` | $O(1)$          | Returns the quantity of active nodes (they behave equally).                                                                |
| `bytes(cont)`                 | $O(\log N)$*    | Returns the total allocated heap memory. *May be $O(N)$, depends on allocation strategy.                                 |
| `is_empty(cont)`              | $O(1)$          | Returns `true` if the container is empty.                                                                                  |

### Stack Specific

| Function             | Time Complexity | Description                                                                                                                            |
|:---------------------|:----------------|:---------------------------------------------------------------------------------------------------------------------------------------|
| `push(stack,⠀value)` | $O(1)$*         | Pushes a value to the top of the stack. *May trigger stack growth.                                                                   |
| `pop(stack,⠀&out)`   | $O(1)$          | Pops the value from the top of the stack. Ownership is transferred to `out`. If `NULL` is passed, the value is automaticaly destroyed. |

### Queue Specific

| Function                | Time Complexity | Description                                                                                                                                 |
|:------------------------|:----------------|:--------------------------------------------------------------------------------------------------------------------------------------------|
| `enqueue(queue,⠀value)` | $O(1)$*         | Inserts a value at the end of the queue. *May trigger queue growth.                                                                       |
| `dequeue(queue,⠀&out)`  | $O(1)$          | Removes the value from the front of the queue. Ownership is transferred to `out`. If `NULL` is passed, the value is automaticaly destroyed. |

### List Specific

| Function                                           | Time Complexity | Description                                                                                                                                                                               |
|:---------------------------------------------------|:----------------|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `push_front(list,⠀value)` / `prepend(list,⠀value)` | $O(1)$*         | Inserts a value at the beginning of the list (they behave equally). *May trigger list growth.                                                                                           |
| `push_back(list,⠀value)` / `append(list,⠀value)`   | $O(1)$*         | Inserts a value at the end of the list (they behave equally). *May trigger list growth.                                                                                                 |
| `push_at(list,⠀index,⠀value)`                      | $O(N)$*         | Inserts a value at the given index within the range $[0, N]$. * $O(1)$ if index == length. May trigger list growth.                                                                       |
| `set_front(list,⠀value)`                           | $O(1)$          | Replaces the first element. Destroys the old value.                                                                                                                                       |
| `set_back(list,⠀value)`                            | $O(1)$          | Replaces the last element. Destroys the old value.                                                                                                                                        |
| `set_at(list,⠀index,⠀value)`                       | $O(N)$          | Replaces the element at the specified index within the range $[0, N)$. Destroys the old value.                                                                                            |
| `pop_front(list,⠀&out)`                            | $O(1)$          | Removes the first element. Ownership is transferred to `out`. If `NULL` is passed, the value is automatically destroyed.                                                                  |
| `pop_back(list,⠀&out)`                             | $O(N)$          | Removes the last element. Ownership is transferred to `out`. If `NULL` is passed, the value is automatically destroyed.                                                                   |
| `pop_at(list,⠀index,⠀&out)`                        | $O(N)$          | Removes the element at the specified index within the range $[0, N)$. Ownership is transferred to `out`. If `NULL` is passed, the value is automatically destroyed.                       |
| `drop_front(list)`                                 | $O(1)$          | Discards the first element. Acts exactly as `pop_front(list, NULL)`.                                                                                                                      |
| `drop_back(list)`                                  | $O(N)$          | Discards the last element. Acts exactly as `pop_back(list, NULL)`.                                                                                                                        |
| `drop_at(list,⠀index)`                             | $O(N)$          | Discards the element at the specified index within the range $[0, N)$. Acts exactly as `pop_at(list, index, NULL)`.                                                                       |

## Container Structure

The generated structures wrap the underlying node chain:

```c++
typedef struct ContainerType
{                                                
    const ds_copier_fn      copy;                                           
    const ds_destructor_fn  destroy;                                        
    struct ds_node_chain    *_nodes;    
} ContainerType;
```

The `copy` and `destroy` fields are `const` intentionally. This prevents unsafe direct assignments (`list1 = list2;`),
which would cause double-frees and memory leaks. To duplicate a container, you must use the generated `copy()` function.

_Warning: Modifying `_nodes` manually invokes **Undefined Behavior**._

---

## Memory Layout

This implementation requires C11 to leverage the `_Alignof()` operator. Instead of allocating separate memory blocks for
node metadata and user data (`void *data`), the library uses an intrusive memory layout.

Each node stride (header + padding + payload) is packed into a single allocation:
``` 
        Node
+---------------------+
| Node Header         |  -> Pointer to next node
+---------------------+
| Padding             |  -> Ensures payload alignment
+---------------------+
| Payload             |  -> User data
+---------------------+
| Padding             |  -> Aligns next node
+---------------------+
```

The node stride is aligned to the stricter of `alignof(Type)` and `alignof(Node)`.
Since Node contains a pointer, its alignment is typically equal to `alignof(void*)`.

_Note: Alignments must be powers of two and cannot exceed `alignof(max_align_t)`._

### Example Layouts (x86-64)

For instance, assuming typical values on x86-64 architectures:
- `alignof(char) == 1`,
- `alignof(int) == 4`,
- `alignof(void*) == 8`,
- `alignof(long double) == 16`,

with corresponding sizes equal to these alignments, the memory layout of the nodes and their data would be organized as follows:

#### `int`
```
       Node
+------------------+
| next pointer (8) |
+------------------+
| int value    (4) |
| padding      (4) |
+------------------+
```

### `pointer`
```
       Node
+------------------+
| next pointer (8) |
+------------------+
| void *value  (8) |
+------------------+
```

`long double`
```
       Node
+------------------+
| next pointer (8) |
+------------------+
| padding      (8) |
+------------------+
| long double (16) |
+------------------+
```

`char buffer[16]`
```
       Node
+------------------+
| next pointer (8) |
+------------------+
| char buffer (16) |
+------------------+
```

No unnecessary padding is introduced, only what alignment strictly requires.

## Memory Allocation Strategy

To prevent excessive heap fragmentation, memory is allocated in contiguous chunks (batches) and sliced into node slots.
Deleted nodes are pushed to an internal recycle stack for $O(1)$ reallocation.

When the recycle stack is empty, the library triggers a geometric heap allocation ($O(\log N)$ frequency) based on two configurable macros:

- `LIBDS_NC_MIN_BATCH_SIZE`: The lower bound on how many nodes are allocated during early usage (default to `8`).
- `LIBDS_NC_GROWTH_FACTOR`: Geometric multiplier for scaling (default to `0.5f`).

The effective batch size is calculated as:

### $B_n = \max(B_{\min} , G \cdot N_n)$

where:
- $B_n$ is the allocated size at step $n$,
- $B_{\min}$ is the minimum allowed batch size (lower bound), 
- $G$ is the growth factor, and
- $N_n$ is the current number of active elements.


Growth progression examples based on configuration:

#### Default $(B_{\min} = 8,  G = \frac{1}{2})$

$8 \to 16 \to 24 \to 36 \to 54 \to 81 \to \dots$

#### Doubling $(B_{\min} = 1,  G = 1)$

$1 \to  2 \to  4 \to  8 \to 16 \to 32 \to \dots$

#### Linear $(B_{\min} = 16,  G = 0)$

$16 \to 32 \to 48 \to 64 \to 80 \to 96 \to \dots$

_Note: the growth is proportional to the current number of elements, not the previous batch size._

## Error Handling

The library provides configurable macros to assist with debugging.

- `LIBDS_ENABLE_ERROR_PRINT` (Default to `1`)
When enabled, internal errors are intercepted and reported to `stderr` with contextual diagnostics.
Disabling this removes logging overhead, but functions will still silently return `enum ds_error` codes.


- `LIBDS_ENABLE_EXIT_ON_FAIL` (Default to `0`)
When enabled, the library will immediately terminate the program via `exit(EXIT_FAILURE)` whenever a failure state is reached.
