#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

typedef struct process_s {
    int uid;
    int uso; 
} processo_t;

typedef struct barrier_s {
    int n;
    int counter;
    // definir aqui o que vai na struct
    // podendo usar semaforo do SO
    // e outras variaveis necessarias
    pthread_mutex_t lock;
    sem_t semaphore;
} barrier_t;

typedef struct fifoQ_s {
    pthread_mutex_t lock;
    sem_t waitSem;
    int count;
    int nextToRelease;
} FifoQT; 

barrier_t* barr;
FifoQT fila;

void init_barr(barrier_t* barr, int n) {
    barr->n = n;
    barr->counter = 0;
    pthread_mutex_init(&(barr->lock), NULL);
    sem_init(&barr->semaphore, 1, 0);
}

void process_barrier(barrier_t* barr) {
    if (!pthread_mutex_lock(&(barr->lock))) {
        barr->counter++;
        pthread_mutex_unlock(&(barr->lock));
    }

    printf("Barrier counter: %d\n", barr->counter);
    if (barr->counter == barr->n) {
        for (int i = 0; i < barr->n; i++) {
            printf("Liberando...\n");
            sem_post(&(barr->semaphore));
        }
    }

    sem_wait(&barr->semaphore);
}

void espera(FifoQT* F) {
    if (!pthread_mutex_lock(&F->lock)) {
        int myPosition = F->count++;
        pthread_mutex_unlock(&F->lock);

        while (1) {
            if (!pthread_mutex_lock(&F->lock)) {
                if (F->nextToRelease == myPosition) {
                    printf("oiii\n");
                    break;
                }
                pthread_mutex_unlock(&F->lock);
                sem_wait(&F->waitSem);
            }
        }
    }
    printf("Saindo do espera\n");
}

void liberaPrimeiro(FifoQT* F) {
    if (!pthread_mutex_lock(&F->lock)) {
        F->nextToRelease++;
        if (F->count > 0) {
            F->count--;
            sem_post(&F->waitSem);
        }
        pthread_mutex_unlock(&F->lock);
    }
}

void init_fifoQ(FifoQT* F) {
    pthread_mutex_init(&F->lock, NULL);
    sem_init(&F->waitSem, 0, 1);
    F->count = 0;
    F->nextToRelease = 0;
}

void processo(int nProc) {
    srand(time(NULL) + nProc);

    int sleep_time = rand() % 4 + 1;
    printf("Processo: %d, Pai: %d, nProc: %d\n", getpid(), getppid(), nProc);
    printf("Processo %d vai dormir por %d segundos\n", nProc, sleep_time);
    sleep(sleep_time);
    process_barrier(barr);
    printf("Processo %d saiu da barreira\n", nProc);

    for (int uso = 0; uso < 3; uso++) {
        // (A) Prologo
        int tempoPrologo = rand() % 4;
        printf( "Processo: %d Prologo: %d de %d segundos\n", nProc, uso, tempoPrologo);
        sleep(tempoPrologo);
        espera(&fila);

        // (B) Uso do recurso
        int tempoUso = rand() % 4;
        printf( "Processo: %d USO: %d por %d segundos\n", nProc, uso, tempoUso);
        sleep(tempoUso);
        liberaPrimeiro(&fila);

        // (C) Epílogo
        int tempoEpilogo = rand() % 4;
        printf( "Processo: %d Epilogo: %d de %d segundos\n", nProc, uso, tempoEpilogo);
        sleep(tempoEpilogo);

        if (nProc != 0) {
            printf("Processo %d terminando\n", nProc);
            exit(0);
        }
    }
}

int main(int argc, char* argv[]) {
    int n;

    if (argc != 2) {
         printf("Usage: %s <nTotalProcesses>\n", argv[0]); 
         return 0;
    } else {
        n = atoi(argv[1]);
    }

    // -- INICIALIZAÇÃO DA SHARED MEMORY

    // chamada de sistema para gerar uma chave única
    key_t key = ftok("shmfile", 65);

    // shmget returns an identifier in shmid
    int shmid = shmget(key, sizeof(barrier_t), 0666 | IPC_CREAT);

    // shmat to attach to shared memory
    // retorna o endereço inicial do segmento de memória
    barr = (barrier_t*)shmat(shmid, (void*)0, 0);

    // -- INICIALIZAÇÃO DA BARREIRA E DA FILA
    init_barr(barr, n);

    // FifoQT fila;
    init_fifoQ(&fila);

    // -- CRIAÇÃO DE PROCESSOS
    for (int i = 1; i < n; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            processo(i);
        }
    }
    processo(0);

    // for (int i = 0; i < n; i++) {
    //     wait(NULL);
    // }

    // -- DEINIT

    sem_destroy(&barr->semaphore);
    sem_destroy(&fila.waitSem);

    // detach from shared memory
    shmdt(barr);

    // libera shared memory
    shmctl(shmid, IPC_RMID, NULL);
}