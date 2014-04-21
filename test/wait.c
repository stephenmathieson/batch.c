
#include <stdlib.h>
#include <unistd.h> // usleep
#include "batch.h"
#include "describe.h"

int count = 0;

void *
task(void *nil) {
  usleep(500);
  count++;
  return NULL;
}

describe("batch_wait", {
  it("should wait for all tasks to complete", {
    batch_t *batch = batch_new(10);
    assert(batch);

    for (int i = 0; i < 20; i++) {
      assert(0 == batch_push(batch, task, NULL));
    }

    assert(0 == batch_wait(batch));
    assert_equal(20, count);

    assert(0 == batch_end(batch));
  });
});
