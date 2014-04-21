
#include <assert.h>
#include <stdio.h>
#include <pthread.h> // pthread_self
#include <unistd.h> // sleep
#include "batch.h"

int count = 0;

int
is_even(int n) {
  return 0 == n % 2 ? 1 : 0;
}

void *
function_even(void *arg) {
  int n = (int) arg;
  assert(is_even(n));
  printf("thread 0x%x: EVEN  <-->  %d\n", (int) pthread_self(), n);
  count++;
  sleep(1);
  return NULL;
}

void *
function_odd(void *arg) {
  int n = (int) arg;
  assert(0 == is_even(n));
  printf("thread 0x%x: ODD   <-->  %d\n", (int) pthread_self(), n);
  count++;
  sleep(1);
  return NULL;
}

int
main() {
  void *batch = batch_new(4);
  assert(batch);

  for (int i = 0; i < 20; ++i) {
    void *(*function)(void *) = is_even(i)
      ? function_even
      : function_odd;
    batch_push(batch, function, (void *) i);
  }

  batch_wait(batch);
  batch_end(batch);

  assert(20 == count);

  return 0;
}
