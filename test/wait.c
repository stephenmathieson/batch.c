
#include <stdlib.h>
#include <unistd.h> // usleep
#include <pthread.h>
#include "batch.h"
#include "describe.h"

int count = 0;
pthread_mutex_t counter_lock;

void *
task(void *nil) {
  usleep(500);
  pthread_mutex_lock(&counter_lock);
  count++;
  pthread_mutex_unlock(&counter_lock);
  return NULL;
}

describe("batch_wait", {
  it("should wait for all tasks to complete", {
    int tasks = 50;
    batch_t *batch = batch_new(10);
    assert(batch);
    pthread_mutex_init(&counter_lock, NULL);

    for (int i = 0; i < tasks; i++) {
      assert(0 == batch_push(batch, task, NULL));
    }

    assert(0 == batch_wait(batch));
    assert_equal(tasks, count);

    assert(0 == batch_end(batch));
  });
});
