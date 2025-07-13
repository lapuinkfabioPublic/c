/*
Fabio Leandro Lapuinka Rede Token Ring
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_ESTACOES 5
#define TEMPO_SIMULACAO 30 // em segundos

typedef struct {
    int id;
    bool possui_token;
    bool deseja_transmitir;
    pthread_mutex_t mutex;
    pthread_cond_t cond_token;
} Estacao;

typedef struct {
    Estacao* estacoes;
    int num_estacoes;
    int estacao_atual;
    bool simulacao_ativa;
} TokenRing;

// Função para passar o token para a próxima estação
void passar_token(TokenRing* ring) {
    pthread_mutex_lock(&ring->estacoes[ring->estacao_atual].mutex);
    ring->estacoes[ring->estacao_atual].possui_token = false;
    pthread_cond_signal(&ring->estacoes[ring->estacao_atual].cond_token);
    pthread_mutex_unlock(&ring->estacoes[ring->estacao_atual].mutex);

    ring->estacao_atual = (ring->estacao_atual + 1) % ring->num_estacoes;

    pthread_mutex_lock(&ring->estacoes[ring->estacao_atual].mutex);
    ring->estacoes[ring->estacao_atual].possui_token = true;
    pthread_cond_signal(&ring->estacoes[ring->estacao_atual].cond_token);
    pthread_mutex_unlock(&ring->estacoes[ring->estacao_atual].mutex);
}

// Função executada por cada estação
void* funcao_estacao(void* arg) {
    Estacao* estacao = (Estacao*)arg;
    
    while (true) {
        pthread_mutex_lock(&estacao->mutex);
        
        // Espera até receber o token
        while (!estacao->possui_token) {
            pthread_cond_wait(&estacao->cond_token, &estacao->mutex);
        }
        
        // Verifica se a simulação ainda está ativa
        TokenRing* ring = (TokenRing*)estacao->ring;
        if (!ring->simulacao_ativa) {
            pthread_mutex_unlock(&estacao->mutex);
            break;
        }
        
        // Se deseja transmitir, faz a transmissão
        if (estacao->deseja_transmitir) {
            printf("Estação %d transmitindo dados...\n", estacao->id);
            sleep(1 + rand() % 2); // Simula tempo de transmissão
            estacao->deseja_transmitir = false;
        } else {
            printf("Estação %d recebeu o token mas não tem dados para transmitir\n", estacao->id);
            sleep(1); // Tempo mínimo com o token
        }
        
        pthread_mutex_unlock(&estacao->mutex);
        
        // Passa o token para a próxima estação
        passar_token(ring);
    }
    
    return NULL;
}

// Função para inicializar a rede Token Ring
TokenRing* iniciar_token_ring(int num_estacoes) {
    TokenRing* ring = (TokenRing*)malloc(sizeof(TokenRing));
    ring->estacoes = (Estacao*)malloc(num_estacoes * sizeof(Estacao));
    ring->num_estacoes = num_estacoes;
    ring->estacao_atual = 0;
    ring->simulacao_ativa = true;
    
    for (int i = 0; i < num_estacoes; i++) {
        ring->estacoes[i].id = i;
        ring->estacoes[i].possui_token = (i == 0);
        ring->estacoes[i].deseja_transmitir = false;
        ring->estacoes[i].ring = ring;
        pthread_mutex_init(&ring->estacoes[i].mutex, NULL);
        pthread_cond_init(&ring->estacoes[i].cond_token, NULL);
    }
    
    return ring;
}

// Função para encerrar a rede Token Ring
void encerrar_token_ring(TokenRing* ring) {
    ring->simulacao_ativa = false;
    
    // Garante que todas as estações são notificadas para encerrar
    for (int i = 0; i < ring->num_estacoes; i++) {
        pthread_mutex_lock(&ring->estacoes[i].mutex);
        ring->estacoes[i].possui_token = true;
        pthread_cond_signal(&ring->estacoes[i].cond_token);
        pthread_mutex_unlock(&ring->estacoes[i].mutex);
    }
    
    // Libera recursos
    for (int i = 0; i < ring->num_estacoes; i++) {
        pthread_mutex_destroy(&ring->estacoes[i].mutex);
        pthread_cond_destroy(&ring->estacoes[i].cond_token);
    }
    
    free(ring->estacoes);
    free(ring);
}

int main() {
    srand(time(NULL));
    
    printf("Iniciando simulação de Token Ring com %d estações\n", NUM_ESTACOES);
    TokenRing* ring = iniciar_token_ring(NUM_ESTACOES);
    
    // Cria threads para cada estação
    pthread_t threads[NUM_ESTACOES];
    for (int i = 0; i < NUM_ESTACOES; i++) {
        pthread_create(&threads[i], NULL, funcao_estacao, &ring->estacoes[i]);
    }
    
    // Simulação: ocasionalmente faz as estações desejarem transmitir
    time_t start_time = time(NULL);
    while (difftime(time(NULL), start_time) < TEMPO_SIMULACAO) {
        sleep(2);
        
        // Escolhe uma estação aleatória para desejar transmitir
        int estacao = rand() % NUM_ESTACOES;
        pthread_mutex_lock(&ring->estacoes[estacao].mutex);
        ring->estacoes[estacao].deseja_transmitir = true;
        printf("Estação %d agora deseja transmitir dados\n", estacao);
        pthread_mutex_unlock(&ring->estacoes[estacao].mutex);
    }
    
    // Encerra a simulação
    encerrar_token_ring(ring);
    
    // Espera todas as threads terminarem
    for (int i = 0; i < NUM_ESTACOES; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Simulação de Token Ring encerrada\n");
    return 0;
}
