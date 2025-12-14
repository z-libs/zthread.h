
# zthread.h

`zthread.h` brings **portable multithreading** to C, with C++ native-like support. It abstracts Win32 and POSIX threads into a unified, type-safe API, allowing you to write concurrent code that runs natively on Windows, Linux, and macOS without complex build configurations.

It is designed to be **zero friction** - handling the differences between `pthreads` and `CreateThread` internally so you can focus on your logic.

## Features

* **Cross-Platform**: Native backends for Win32 and POSIX (pthread). No middleware or heavy runtimes.
* **Type-Safe Creation**: Macros automatically handle `void*` casting, allowing typed function arguments.
* **Unified Primitives**: Consistent API for Mutexes and Condition Variables across all OSs.
* **Strict Compliance**: Optional `ZTHREAD_WRAP` macro for pedantic standard compliance (avoids function pointer casting).
* **Zero Dependencies**: Uses only standard system headers.
* **ZDK Integration**: Respects global ZDK memory allocators (`Z_MALLOC`) for internal allocations.
* **Single Header**: Drop-in and use. No linking arguments (`-lpthread` is strictly optional on modern GCC/Clang, but recommended).

## Installation

1.  Copy `zthread.h` to your project's include folder.
2.  Include it where needed.

```c
#define ZTHREAD_IMPLEMENTATION // Define in ONE .c file.
#include "zthread.h"
```

> **Optional Short Macros:** To enable clean keywords like `thread_create`, `mutex_lock`, and `cond_wait`, define `ZTHREAD_SHORT_NAMES` before including the header.

```c
#define ZTHREAD_SHORT_NAMES
#include "zthread.h"
```
## Quick Start


### Creating a Thread

Instead of casting `void*` arguments manually, `zthread.h` lets you pass typed pointers directly.

```c
void worker(int *counter) 
{
  (*counter)++;
  printf("Counter updated to: %d\n", *counter);
}

int main(void) 
{
    zthread_t t;
    int count = 0;

    // Macro automatically casts 'worker' and '&count'.
    if (zthread_create(&t, worker, &count) == Z_OK) 
    {
        zthread_join(t);
    }
    return 0;
}
```

### Synchronization (Mutexes)

Protect shared resources using `zmutex`. The API is unified for Windows and Linux.

```c
zmutex_t lock;
int shared_resource = 0;

void unsafe_increment() 
{
    zmutex_lock(&lock);
    shared_resource++;  // Critical section.
    zmutex_unlock(&lock);
}

int main(void) 
{
    zmutex_init(&lock);
    // ... spawn threads ...
    zmutex_destroy(&lock);
}
```

### Coordination (Condition Variables)

Wait for signals efficiently without busy loops.

```c
zmutex_t m;
zcond_t ready;
bool data_is_ready = false;

void consumer() 
{
    zmutex_lock(&m);
    while (!data_is_ready) 
    {
        // Releases lock and sleeps until signaled.
        zcond_wait(&ready, &m);
    }
    process_data();
    zmutex_unlock(&m);
}

void producer() 
{
    zmutex_lock(&m);
    data_is_ready = true;
    zcond_signal(&ready); // Wake up the consumer
    zmutex_unlock(&m);
}
```

## Advanced Usage

### Strict Wrappers (`ZTHREAD_WRAP`)

While casting function pointers is common in C, strict standard compliance technically forbids casting `void (*)(T*)` to `void (*)(void*)`. If you need 100% compliance, use the wrapper generator.

```c
// Generates a strictly compliant void* proxy function.
ZTHREAD_WRAP(heavy_task, Config*, cfg) 
{
    printf("Processing %s\n", cfg->name);
}

int main(void) 
{
    Config c = { "Job 1" };
    zthread_t t;
        
    // Pass the safe wrapper to the thread creator.
    zthread_create(&t, heavy_task, &c); 
}
```

### Custom Allocators

`zthread.h` allocates a tiny wrapper struct for every thread to bridge the OS entry point types. By default, it uses `malloc`. You can override this globally or locally. You can use `zalloc.h`.

```c
// Override for just zthread
#define ZTHREAD_MALLOC(sz) my_custom_alloc(sz)
#define ZTHREAD_FREE(ptr)  my_custom_free(ptr)
#include "zthread.h"
```

## API Reference (C)

**Thread Management**

| Function/Macro | Description |
| :--- | :--- |
| `zthread_create(t, fn, arg)` | Spawns a new thread. Returns `Z_OK` on success. |
| `zthread_join(t)` | Blocks until the thread `t` finishes execution. |
| `zthread_detach(t)` | Detaches the thread (it cleans up automatically on exit). |
| `zthread_sleep(ms)` | Sleeps the current thread for `ms` milliseconds. |
| `ZTHREAD_WRAP(name, T, v)` | Defines a type-safe wrapper implementation block. |

**Synchronization**

| Function | Description |
| :--- | :--- |
| `zmutex_init(m)` | Initializes a mutex. |
| `zmutex_lock(m)` | Acquires the lock (blocks if taken). |
| `zmutex_unlock(m)` | Releases the lock. |
| `zmutex_destroy(m)` | Frees mutex resources. |

**Condition Variables**

| Function | Description |
| :--- | :--- |
| `zcond_init(c)` | Initializes a condition variable. |
| `zcond_wait(c, m)` | Atomically unlocks mutex `m` and waits for signal on `c`. |
| `zcond_signal(c)` | Wakes up **one** waiting thread. |
| `zcond_broadcast(c)` | Wakes up **all** waiting threads. |
| `zcond_destroy(c)` | Frees condition variable resources. |

## API Reference (C++)

The C++ wrapper lives in the **`z_thread`** namespace. It strictly adheres to RAII principles and delegates all logic to the underlying C implementation.

### `class z_thread::thread`

**Constructors & Management**

| Method | Description |
| :--- | :--- |
| `thread()` | Default constructor (empty/inactive). |
| `thread(Func&& f, Args&&...)` | Spawns a new thread executing `f` with arguments. |
| `~thread()` | Destructor. Terminates if thread is still joinable. |
| `operator=` | Move assignment operator. |

**Control & State**

| Method | Description |
| :--- | :--- |
| `join()` | Blocks until the thread finishes execution. |
| `detach()` | Detaches the thread (runs independently). |
| `joinable_state()` | Returns `true` if the thread is active and joinable. |
| `native_handle()` | Returns the underlying `zthread_t` handle. |
| `sleep(ms)` | **Static**. Sleeps the current thread for `ms` milliseconds. |

### `class z_thread::mutex`

**Management & Locking**

| Method | Description |
| :--- | :--- |
| `mutex()` | Default constructor. Initializes the mutex. |
| `~mutex()` | Destructor. Destroys the mutex resources. |
| `lock()` | Acquires the lock (blocks if already taken). |
| `unlock()` | Releases the lock. |
| `native_handle()` | Returns pointer to underlying `zmutex_t`. |

### `class z_thread::lock_guard`

**RAII Locking**

| Method | Description |
| :--- | :--- |
| `lock_guard(mutex& m)` | Acquires lock on construction. |
| `~lock_guard()` | Releases lock on destruction. |

### `class z_thread::cond`

**Waiting & Signaling**

| Method | Description |
| :--- | :--- |
| `cond()` | Default constructor. Initializes the condition variable. |
| `~cond()` | Destructor. Destroys the condition variable. |
| `wait(mutex& m)` | Atomically unlocks `m` and waits for a signal. Relocks on return. |
| `signal()` | Wakes up **one** waiting thread. |
| `broadcast()` | Wakes up **all** waiting threads. |
| `native_handle()` | Returns pointer to underlying `zcond_t`. |

## Configuration Options

| Define | Effect |
| :--- | :--- |
| `ZTHREAD_IMPLEMENTATION` | Enables the implementation (define in one `.c` file). |
| `ZTHREAD_SHORT_NAMES` | Enables short aliases (`thread_create`, `mutex_lock`, etc.). |
| `ZTHREAD_MALLOC` | Override memory allocation (Default: `stdlib.h` malloc). |
| `ZTHREAD_FREE` | Override memory free (Default: `stdlib.h` free). |
