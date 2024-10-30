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
#include <time.h>

// Barreira
typedef struct barrier_s {
    int n; // Número total de processos
    int counter; // Contador de 0 até n
    pthread_mutex_t lock; // Protege as variáveis da barreira
    sem_t semaphore; // Controla a liberação dos processos
} barrier_t;

// Fila
typedef struct fifoQ_s {
    pthread_mutex_t lock; // Protege as variáveis da barreira
    int count; // Contagem de processos colocados na fila
    int next; // Próximo a ser liberado
} FifoQT; 

// Retorna horário atual do sistema
char* get_current_time() {
    static char time_str[9];
    time_t now = time(NULL);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", localtime(&now));
    return time_str;
}

// Inicializa a barreira
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

    if (barr->counter == barr->n) {
        for (int i = 0; i < barr->n; i++) {
            sem_post(&(barr->semaphore));
        }
        printf("\n");
    }

    sem_wait(&barr->semaphore);
}

void espera(FifoQT* F, int nProc) {
    printf("%s - Processo %d entrando na fila...\n", get_current_time(), nProc);
    int posicao;
    if (!pthread_mutex_lock(&(F->lock))) {
        posicao = F->count++;
        pthread_mutex_unlock(&(F->lock));
        printf("%s - Processo: %d, Posição: %d\n", get_current_time(), nProc, posicao);
    }

    while (1) {
        if (F->proximo == posicao) {
            break;
        }
    }
    printf("%s - Processo %d saindo da fila...\n", get_current_time(), nProc);
}

void liberaPrimeiro(FifoQT* F, int nProc) {
    if (!pthread_mutex_lock(&F->lock)) {
        F->proximo++;
        pthread_mutex_unlock(&(F->lock));
    }
    printf("%s - Processo %d liberando a fila...\n", get_current_time(), nProc);
}

void init_fifoQ(FifoQT* F) {
    pthread_mutex_init(&F->lock, NULL);
    sem_init(&F->waitSem, 0, 1);
    F->count = 0;
    F->proximo = 0;
}

void processo(int nProc) {
    srand(time(NULL) + nProc);

    int sleep_time = rand() % 4 + 1;
    printf("%s - Processo: %d, Pai: %d, nProc: %d\n", get_current_time(), getpid(), getppid(), nProc);
    printf("%s - Processo %d vai dormir por %d segundos\n", get_current_time(), nProc, sleep_time);
    sleep(sleep_time);
    process_barrier(barr);
    printf("%s - Processo %d saiu da barreira\n", get_current_time(), nProc);

    for (int uso = 0; uso < 3; uso++) {
        // (A) Prólogo
        int tempoPrologo = rand() % 4;
        printf("%s - Processo: %d Prólogo: %d de %d segundos\n", get_current_time(), nProc, uso, tempoPrologo);
        sleep(tempoPrologo);
        espera(fila, nProc);

        // (B) Uso do recurso
        int tempoUso = rand() % 4;
        printf("%s - Processo: %d Uso: %d por %d segundos\n", get_current_time(), nProc, uso, tempoUso);
        sleep(tempoUso);
        liberaPrimeiro(fila, nProc);

        // (C) Epílogo
        int tempoEpilogo = rand() % 4;
        printf("%s - Processo: %d Epílogo: %d de %d segundos\n", get_current_time(), nProc, uso, tempoEpilogo);
        sleep(tempoEpilogo);
    }
}

// Variáveis globais

barrier_t* barr;
FifoQT* fila;

int main(int argc, char* argv[]) {
    int n;

    if (argc != 2) {
         printf("%s - Usage: %s <nTotalProcesses>\n", get_current_time(), argv[0]); 
         return 0;
    } else {
        n = atoi(argv[1]);
    }

    // -- INICIALIZAÇÃO DA SHARED MEMORY

    // chamada de sistema para gerar uma chave única
    // key_t barr_key = ftok("makefile", 65);
    // key_t fifo_key = ftok("makefile", 65);
    key_t barr_key = 2378;
    key_t fifo_key = 2379;

    // printf("%d %d\n", barr_key, fifo_key);

    // shmget returns an identifier in shmid
    int barr_shmid = shmget(barr_key, sizeof(barrier_t), 0666 | IPC_CREAT);
    int fifo_shmid = shmget(fifo_key, sizeof(FifoQT), 0666 | IPC_CREAT);

    // shmat to attach to shared memory
    // retorna o endereço inicial do segmento de memória
    barr = (barrier_t*) shmat(barr_shmid, (void*)0, 0);
    fila = (FifoQT*) shmat(fifo_shmid, (void*)0, 0);

    // -- INICIALIZAÇÃO DA BARREIRA E DA FILA
    init_barr(barr, n);

    init_fifoQ(fila);

    // -- CRIAÇÃO DE PROCESSOS
    for (int i = 1; i < n; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            processo(i);
            exit(0);
        }
    }
    processo(0);

    for (int i = 0; i < n-1; i++) {
        wait(NULL);
    }

    // -- DEINIT

    sem_destroy(&barr->semaphore);
    sem_destroy(&fila->waitSem);

    // detach from shared memory
    shmdt(barr);
    shmdt(fila);

    // libera shared memory
    shmctl(barr_shmid, IPC_RMID, NULL);
    shmctl(fifo_shmid, IPC_RMID, NULL);

    printf("%s - Processo 0 terminando\n", get_current_time());
}