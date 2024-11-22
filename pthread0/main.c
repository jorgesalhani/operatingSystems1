#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

typedef struct {
  int idx, length;
} thread_arg, *ptr_thread_arg;

pthread_t threads[2];

void *thread_func(void *arg) {
  ptr_thread_arg targ = (ptr_thread_arg) arg;
  
  for (int i = targ->idx; i < (targ->idx + targ->length); i++) {
    printf("Thread %d - value %d\n", pthread_self(), i);
  }
}

int main(int argc, char **argv) {
  thread_arg arguments[2];

  for (int i = 0; i < 2; i++) {
    arguments[i].idx = i*10;
    arguments[i].length = 10;
    pthread_create(&(threads[i]), NULL, thread_func, &(arguments[i]));
  }

  for (int i = 0; i < 2; i++) {
    pthread_join(threads[i], NULL);
  }
}