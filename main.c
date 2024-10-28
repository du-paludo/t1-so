#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

typedef struct barrier_s {
    int n;
    int counter;
    // definir aqui o que vai na struct
    // podendo usar semaforo do SO
    // e outras variaveis necessarias
    pthread_mutex_t lock;
    sem_t semaphore;
} barrier_t;

void init_barr(barrier_t* barr, int n, char* str) {
    barr->n = n;
    barr->counter = 0;
    pthread_mutex_init(&(barr->lock), NULL);
    sem_init(barr->semaphore, 1, barr->n);
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
    }

    sem_wait(barr->semaphore);
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
    barrier_t* barr = (barrier_t*)shmat(shmid, (void*)0, 0);

    // -- INICIALIZAÇÃO DA BARREIRA
    init_barr(barr, 8, barr);

    // -- CRIAÇÃO DE PROCESSOS
    for (int i = 1; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            int nProc = i;
            int sleep_time = rand() % n;
            printf("Processo: %d, Pai: %d, nProc: %d\n", getpid(), getppid(), nProc);
            printf("Processo %d vai dormir por %d segundos\n", nProc, sleep_time);
            sleep(sleep_time);
            
            process_barrier(barr);
            printf("Processo %d saiu da barreira\n", nProc);
            exit(0);
        }
    }

    int nProc = 0; // Processo pai (nProc=0)
    int sleep_time = rand() % n;
    printf("Processo: %d, Pai: %d, nProc: %d\n", getpid(), getppid(), nProc);
    printf("Processo %d vai dormir por %d segundos\n", nProc, sleep_time);
    sleep(sleep_time);

    process_barrier(barr);  // Processo pai entra na barreira
    printf("Processo %d saiu da barreira\n", nProc);

    // -- DEINIT

    // detach from shared memory
    shmdt(barr);

    // libera shared memory
    shmctl(shmid, IPC_RMID, NULL);
}