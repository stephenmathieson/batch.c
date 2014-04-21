
# batch.c

  A tiny async batch thingy using POSIX threads.  Asynchronously execute callbacks in parallel with built-in concurrency control.

## API

```c
/**
 * Create a new batch of `concurrency` threads.
 */

batch_t *
batch_new(unsigned int concurrency);

/**
 * Push a callback into the batch queue.
 *
 * Returns BATCH_SUCCESS (0) on success.
 */

batch_error_t
batch_push(batch_t *self, void *(*fn)(void *), void *arg);

/**
 * Wait for all callbacks in the queue to complete.
 *
 * Returns BATCH_SUCCESS (0) on success.
 */

batch_error_t
batch_wait(batch_t *self);

/**
 * Stop all batch threads.  Will block the main thread
 * until all worker threads have completed.  Frees all
 * memory associated with the batch on success.
 *
 * Returns BATCH_SUCCESS (0) on success.
 */

batch_error_t
batch_end(batch_t *self);

/**
 * Get a human-readable string from the given batch
 * error `code`.
 */

const char *
batch_error_string(batch_error_t code);
```

## License

  MIT
