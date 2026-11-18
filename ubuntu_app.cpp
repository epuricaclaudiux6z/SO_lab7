#include <iostream>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

enum Color { NONE, WHITE, BLACK };
Color color_using = NONE;
Color turn = NONE;

int active_W = 0, active_B = 0;
int waiting_W = 0, waiting_B = 0;

void* white_thread(void* arg) {
    int id = *(int*)arg;
    
    pthread_mutex_lock(&mtx);
    waiting_W++;
    if (color_using == BLACK) turn = WHITE; // Cerem rândul
    
    // Așteptăm dacă resursa e folosită de negri sau dacă sunt negri în așteptare și e rândul lor
    while (color_using == BLACK || (waiting_B > 0 && turn == BLACK)) {
        pthread_cond_wait(&cv, &mtx);
    }
    
    waiting_W--;
    active_W++;
    color_using = WHITE;
    pthread_mutex_unlock(&mtx);

    // Secțiunea critică
    std::cout << "[Ubuntu] Firul ALB " << id << " foloseste resursa.\n";
    sleep(1);

    pthread_mutex_lock(&mtx);
    active_W--;
    if (active_W == 0) {
        color_using = NONE;
        pthread_cond_broadcast(&cv); // Trezește toate firele (inclusiv pe cele negre)
    }
    pthread_mutex_unlock(&mtx);
    
    return nullptr;
}

void* black_thread(void* arg) {
    int id = *(int*)arg;
    
    pthread_mutex_lock(&mtx);
    waiting_B++;
    if (color_using == WHITE) turn = BLACK; // Cerem rândul
    
    while (color_using == WHITE || (waiting_W > 0 && turn == WHITE)) {
        pthread_cond_wait(&cv, &mtx);
    }
    
    waiting_B--;
    active_B++;
    color_using = BLACK;
    pthread_mutex_unlock(&mtx);

    // Secțiunea critică
    std::cout << "[Ubuntu] Firul NEGRU " << id << " foloseste resursa.\n";
    sleep(1);

    pthread_mutex_lock(&mtx);
    active_B--;
    if (active_B == 0) {
        color_using = NONE;
        pthread_cond_broadcast(&cv);
    }
    pthread_mutex_unlock(&mtx);
    
    return nullptr;
}

int main() {
    pthread_t threads[6];
    int ids[6] = {1, 2, 3, 4, 5, 6};

    // Creăm firele intercalat pentru a testa mecanismul anti-înfometare
    pthread_create(&threads[0], NULL, white_thread, &ids[0]);
    pthread_create(&threads[1], NULL, black_thread, &ids[1]);
    pthread_create(&threads[2], NULL, white_thread, &ids[2]);
    pthread_create(&threads[3], NULL, black_thread, &ids[3]);
    pthread_create(&threads[4], NULL, white_thread, &ids[4]);
    pthread_create(&threads[5], NULL, black_thread, &ids[5]);

    for(int i = 0; i < 6; i++) {
        pthread_join(threads[i], NULL);
    }
    
    std::cout << "[Ubuntu] Executie completă.\n";
    return 0;
}
