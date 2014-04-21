
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "batch.h"

/**
 * A batch task.
 */

struct batch_task {

  /**
   * Argument provided to the callback.
   */

  void *arg;

  /**
   * The callback function.
   */

  void *(*fn)(void *);

  /**
   * The next task.
   */

  struct batch_task *next;
};

struct batch_t {

  /**
   * Cancelled flag.
   */

  int cancelled;

  /**
   * Number of pending tasks.
   */

  unsigned int pending;

  /**
   * Number of tasks to execute in parallel.
   */

  unsigned int concurrency;

  /**
   * Our task queue.
   */

  struct batch_task *queue;

  /**
   * The last task in our queue.
   */

  struct batch_task *end;

  /**
   * Our mutex.
   */

  pthread_mutex_t lock;

  /**
   * Our signal condition.
   */

  pthread_cond_t notify;

  /**
   * Our thread IDs.
   */

  pthread_t threads[1];
};

/**
 * Batch worker thread.  Grabs the next task
 * in the queue and executes it until all
 * tasks have been completed.
 */

static void *
batch_thread(void *arg);

/**
 * Free memory associated with the given batch.
 */

void
batch_free(batch_t *self);

batch_t *
batch_new(unsigned int concurrency) {
  if (1 > concurrency) return NULL;
  // TODO max concurrency

  unsigned int size = sizeof(batch_t) + (concurrency - 1) * sizeof(pthread_t);
  batch_t *self = malloc(size);
  if (NULL == self) goto error;

  if (0 != pthread_mutex_init(&self->lock, NULL)) goto error;
  if (0 != pthread_cond_init(&self->notify, NULL)) goto error;

  self->concurrency = concurrency;
  self->cancelled = 0;
  self->pending = 0;
  self->end = NULL;
  self->queue = NULL;

  for (unsigned int i = 0; i < concurrency; i++) {
    int rc = pthread_create(
        &self->threads[i]
      , NULL
      , &batch_thread
      , self
    );
    if (0 != rc) {
      // kill each thread we've just spawned
      for (unsigned int j = i; j > 0; j--) {
        pthread_join(self->threads[j], NULL);
      }
      goto error;
    }
  }

  return self;

error:
  if (self) batch_free(self);
  self = NULL;
  return self;
}

// TODO: check cancelled
batch_error_t
batch_push(batch_t *self, void *(*fn)(void *), void *arg) {
  struct batch_task *queue = NULL;

  if (NULL == self) return BATCH_INVALID_ERROR;

  if (NULL == (queue = malloc(sizeof(struct batch_task)))) {
    return BATCH_MALLOC_ERROR;
  }

  queue->fn = fn;
  queue->arg = arg;
  queue->next = NULL;

  pthread_mutex_lock(&self->lock);
  if (self->end) self->end->next = queue;
  if (NULL == self->queue) self->queue = queue;

  self->end = queue;
  self->pending++;

  if (0 != pthread_cond_signal(&self->notify)) return BATCH_SIGNAL_ERROR;
  if (0 != pthread_mutex_unlock(&self->lock)) return BATCH_LOCK_ERROR;
  return BATCH_SUCCESS;
}

int
batch_wait(batch_t *self) {
  if (NULL == self) return BATCH_INVALID_ERROR;

  if (0 != pthread_mutex_lock(&self->lock)) return BATCH_LOCK_ERROR;

  // XXX: could race
  while (!self->cancelled && self->pending) {
    if (0 != pthread_cond_wait(&self->notify, &self->lock)) {
      return BATCH_SIGNAL_ERROR;
    }
  }

  if (0 != pthread_mutex_unlock(&self->lock)) return BATCH_LOCK_ERROR;
  return BATCH_SUCCESS;
}

batch_error_t
batch_end(batch_t *self) {
  batch_error_t rc = BATCH_SUCCESS;

  if (NULL == self) return BATCH_INVALID_ERROR;

  self->cancelled = 1;

  do {
    if (0 != pthread_mutex_lock(&self->lock)) {
      rc = BATCH_LOCK_ERROR;
      break;
    }

    if (0 != pthread_cond_broadcast(&self->notify)) {
      rc = BATCH_SIGNAL_ERROR;
      break;
    }

    if (0 != pthread_mutex_unlock(&self->lock)) {
      rc = BATCH_LOCK_ERROR;
      break;
    }

    for (unsigned int i = 0; i < self->concurrency; i++) {
      // even if a join fails, we want to join the rest of our workers
      if (0 != pthread_join(self->threads[i], NULL)) {
        rc = BATCH_JOIN_ERROR;
      }
    }

  } while (0);

  // only if all above conditions passed can we free our
  // memory.  otherwise, blocks (and threads) will be left
  // in limbo
  if (BATCH_SUCCESS == rc) {
    batch_free(self);
    self = NULL;
  }
  return rc;
}

const char *
batch_error_string(batch_error_t code) {
  switch (code) {
    case BATCH_SUCCESS: return NULL;
    case BATCH_MALLOC_ERROR: return "Error allocating memory";
    case BATCH_SIGNAL_ERROR: return "Error notifiing workers";
    case BATCH_LOCK_ERROR: return "Error locking mutex";
    case BATCH_JOIN_ERROR: return "Error joining thread";
    case BATCH_INVALID_ERROR: return "Invalid arguments";
  }
}

void
batch_free(batch_t *self) {
  if (NULL == self) return;
  while (self->queue) {
    struct batch_task *queue = self->queue;
    self->queue = queue->next;
    free(queue);
  }
  free(self);
  self = NULL;
}

// TODO: handle return codes
static void *
batch_thread(void *arg) {
  struct batch_task *queue;
  batch_t *self = (batch_t *) arg;

  while (!self->cancelled) {
    pthread_mutex_lock(&self->lock);
    while (!self->cancelled && self->queue == NULL) {
      pthread_cond_wait(&self->notify, &self->lock);
    }
    if (self->cancelled) {
      pthread_mutex_unlock(&self->lock);
      return NULL;
    }
    queue = self->queue;
    self->queue = queue->next;
    self->end = (queue == self->end ? NULL : self->end);
    pthread_mutex_unlock(&self->lock);

    queue->fn(queue->arg);

    free(queue);
    queue = NULL;

    pthread_mutex_lock(&self->lock);
    self->pending--;
    pthread_cond_broadcast(&self->notify);
    pthread_mutex_unlock(&self->lock);
  }

  return NULL;
}
