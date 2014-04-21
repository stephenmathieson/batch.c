
#ifndef BATCH_H
#define BATCH_H 1

/**
 * Batch type.
 */

typedef struct batch_t batch_t;

/**
 * Error codes.
 */

typedef enum {
    BATCH_SUCCESS = 0
  , BATCH_MALLOC_ERROR = -1
  , BATCH_SIGNAL_ERROR = -2
  , BATCH_LOCK_ERROR = -3
  , BATCH_JOIN_ERROR = -4
  , BATCH_INVALID_ERROR = -5
} batch_error_t;

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

#endif
