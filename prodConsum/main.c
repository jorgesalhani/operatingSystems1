#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <windows.h>
#include <semaphore.h>

#define NUMCONS 2
#define NUMPROD 2
#define BUFFERSIZE 1000

pthread_t cons[NUMCONS];
pthread_t prod[NUMPROD];
int buffer[BUFFERSIZE];
int prod_pos = 0; 
int cons_pos = 0;

void *consumidor(void *arg);
void *produtor(void *arg);

/**
 * Sem mutex - semaforo
 * -- problema de sincronia
 * 
 * 1. consumidor lê buffer com 0
 * 2. escalonado
 * 3. produtor lê buffer com 0
 * 4. produtor escreve buffer com 1
 * 5. escalonado
 * 6. consumidor ainda entende que buffer está vazio
 */

pthread_mutex_t buffer_mutex;
sem_t free_positions;
sem_t filled_positions;

int main(int argc, char **argv) {
  int i;

  srand48(time(NULL));
  pthread_mutex_init(&buffer_mutex, NULL);

  sem_init(&free_positions, 0, BUFFERSIZE);
  sem_init(&filled_positions, 0, 0);

  for (i = 0; i < NUMCONS; i++) {
    pthread_create(&(cons[i]), NULL, consumidor, NULL);
  }

  for (i = 0; i < NUMPROD; i++) {
    pthread_create(&(prod[i]), NULL, produtor, NULL);
  }

  for (i = 0; i < NUMCONS; i++) {
    pthread_join(cons[i], NULL);
  }

  for (i = 0; i < NUMPROD; i++) {
    pthread_join(prod[i], NULL);
  }
}

void *produtor(void *arg) {
  int n;
  while (1) {
    n = (int)(drand48() * 1000.0);

    sem_wait(&free_positions);
    pthread_mutex_lock(&buffer_mutex);

    buffer[prod_pos] = n;
    prod_pos = (prod_pos + 1) % BUFFERSIZE;

    pthread_mutex_unlock(&buffer_mutex);
    sem_post(&filled_positions);

    printf("Produzindo numero %d\n", n);
    Sleep((int)(drand48() * 4.0));
  }
}

void *consumidor(void *arg) {
  int n;
  while(1) {
    sem_wait(&filled_positions);
    pthread_mutex_lock(&buffer_mutex);
    
    n = buffer[cons_pos];
    cons_pos = (cons_pos + 1) % BUFFERSIZE;

    pthread_mutex_unlock(&buffer_mutex);
    sem_post(&free_positions);

    printf("Consumindo numero %d\n", n);
    Sleep((int)(drand48() * 4.0));
  }
}
