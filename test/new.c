
#include <stdlib.h>
#include "batch.h"
#include "describe.h"

describe("batch_new", {
  it("should require positive concurrency", {
    batch_t *batch = batch_new(0);
    assert_null(batch);
  });

  it("should create a new batch", {
    batch_t *batch = batch_new(10);
    assert(batch);
    assert(0 == batch_end(batch));
  });
});
