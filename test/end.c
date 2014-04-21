
#include <stdlib.h>
#include <unistd.h> // usleep
#include "batch.h"
#include "describe.h"

int count = 0;

void *
thread(void *nil) {
  count++;
  usleep(500);
  return NULL;
}

describe("batch_end", {
  batch_t *batch = batch_new(5);
  assert(batch);

  for (int i = 0; i < 20; i++) {
    assert(0 == batch_push(batch, thread, NULL));
  }

  // hackish, but we just want to make sure that *some*
  // of our tasks didn't complete
  it("should force all tasks to end", {
    assert(0 == batch_end(batch));
    assert(count < 20);
  });
});
