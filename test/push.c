
#include <stdlib.h>
#include "batch.h"
#include "describe.h"

int ran_a = 0;
int ran_b = 0;

typedef struct {
  int foo;
} argument;

void *
a(void *nil) {
  ran_a = 1;
  return NULL;
}

void *
b(void *nil) {
  ran_b = 1;
  return NULL;
}

void *
with_argument(void *param) {
  argument *arg = (argument *) param;
  assert(1234 == arg->foo);
  return NULL;
}

describe("batch_push", {
  batch_t *batch = batch_new(2);
  assert(batch);

  it("should add a task to the queue", {
    assert(0 == batch_push(batch, a, NULL));
    assert(0 == batch_push(batch, b, NULL));
    assert(0 == batch_wait(batch));
    assert(1 == ran_a);
    assert(1 == ran_b);
  });

  it("should support an arbitrary argument", {
    argument arg;
    arg.foo = 1234;
    assert(0 == batch_push(batch, with_argument, (void *) &arg));
    assert(0 == batch_wait(batch));
  });

  assert(0 == batch_end(batch));
});
