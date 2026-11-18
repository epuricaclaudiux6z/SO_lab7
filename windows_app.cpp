#include <iostream>
#include <windows.h>

CRITICAL_SECTION cs;
CONDITION_VARIABLE cv;

enum Color { NONE, WHITE, BLACK };
Color color_using = NONE;
Color turn = NONE;

int active_W = 0, active_B = 0;
int waiting_W = 0, waiting_B = 0;

DWORD WINAPI white_thread(LPVOID arg) {
    int id = *(int*)arg;
    
    EnterCriticalSection(&cs);
    waiting_W++;
    if (color_using == BLACK) turn = WHITE;
    
    while (color_using == BLACK || (waiting_B > 0 && turn == BLACK)) {
        SleepConditionVariableCS(&cv, &cs, INFINITE);
    }
    
    waiting_W--;
    active_W++;
    color_using = WHITE;
    LeaveCriticalSection(&cs);

    // Secțiunea critică
    std::cout << "[Windows] Firul ALB " << id << " foloseste resursa.\n";
    Sleep(1000); // 1 secunda

    EnterCriticalSection(&cs);
    active_W--;
    if (active_W == 0) {
        color_using = NONE;
        WakeAllConditionVariable(&cv);
    }
    LeaveCriticalSection(&cs);
    
    return 0;
}

DWORD WINAPI black_thread(LPVOID arg) {
    int id = *(int*)arg;
    
    EnterCriticalSection(&cs);
    waiting_B++;
    if (color_using == WHITE) turn = BLACK;
    
    while (color_using == WHITE || (waiting_W > 0 && turn == WHITE)) {
        SleepConditionVariableCS(&cv, &cs, INFINITE);
    }
    
    waiting_B--;
    active_B++;
    color_using = BLACK;
    LeaveCriticalSection(&cs);

    // Secțiunea critică
    std::cout << "[Windows] Firul NEGRU " << id << " foloseste resursa.\n";
    Sleep(1000);

    EnterCriticalSection(&cs);
    active_B--;
    if (active_B == 0) {
        color_using = NONE;
        WakeAllConditionVariable(&cv);
    }
    LeaveCriticalSection(&cs);
    
    return 0;
}

int main() {
    InitializeCriticalSection(&cs);
    InitializeConditionVariable(&cv);

    HANDLE threads[6];
    int ids[6] = {1, 2, 3, 4, 5, 6};

    threads[0] = CreateThread(NULL, 0, white_thread, &ids[0], 0, NULL);
    threads[1] = CreateThread(NULL, 0, black_thread, &ids[1], 0, NULL);
    threads[2] = CreateThread(NULL, 0, white_thread, &ids[2], 0, NULL);
    threads[3] = CreateThread(NULL, 0, black_thread, &ids[3], 0, NULL);
    threads[4] = CreateThread(NULL, 0, white_thread, &ids[4], 0, NULL);
    threads[5] = CreateThread(NULL, 0, black_thread, &ids[5], 0, NULL);

    WaitForMultipleObjects(6, threads, TRUE, INFINITE);

    for(int i = 0; i < 6; i++) {
        CloseHandle(threads[i]);
    }
    
    DeleteCriticalSection(&cs);
    std::cout << "[Windows] Executie completă.\n";
    return 0;
}
