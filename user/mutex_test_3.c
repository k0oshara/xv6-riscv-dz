#include "kernel/types.h"
#include "user.h"

void test_rw() {
    int mtx = mutex();
    char buf[1];
    
    printf("Test 1: read/write on mutex... ");
    if (read(mtx, buf, 1) < 0) printf("[OK] read failed\n");
    else printf("[FAIL] read succeeded\n");
    
    if (write(mtx, "x", 1) < 0) printf("[OK] write failed\n");
    else printf("[FAIL] write succeeded\n");
    
    close(mtx);
}

void test_close_owner() {
    int mtx = mutex();
    mutex_lock(mtx);
    
    printf("Test 2a: close by owner... ");
    if (close(mtx) < 0) printf("[FAIL] close error\n");
    else printf("[OK] closed\n");
}

void test_close_foreign() {
    int mtx = mutex();
    mutex_lock(mtx);
    int pid = fork();
    
    if (pid == 0) {
        printf("Test 2b: close by other... ");
        int rc = close(mtx);
        if (rc < 0) printf("[OK] close failed (not owner)\n");
        else printf("[FAIL] close succeeded\n");
        exit(0);
    } else {
        wait(0);
        mutex_unlock(mtx);
        close(mtx);
    }
}

void test_exit_with_mutex() {
    int pid = fork();
    if (pid == 0) {
        int mtx = mutex();
        mutex_lock(mtx);
        printf("Child with mutex exiting...\n");
        exit(0);
    }
    wait(0);
    printf("Test 3: process exited with locked mutex\n");

    int mtx = mutex();
    if (mtx >= 0) {
        printf("[OK] mutex allocated after cleanup\n");
        close(mtx);
    } else {
        printf("[FAIL] memory leak detected\n");
    }
}

void test_unlock_foreign() {
    int mtx = mutex();
    mutex_lock(mtx);
    int pid = fork();
    
    if (pid == 0) {
        printf("Test 4: unlock foreign... ");
        int rc = mutex_unlock(mtx);
        if (rc < 0) printf("[OK] unlock failed\n"); 
        else printf("[FAIL] unlock succeeded\n");
        exit(0);
    } else {
        wait(0);
        printf("Test 4: parent unlock check... ");
        int rc = mutex_unlock(mtx);
        if (rc == 0) printf("[OK] parent unlocked\n");
        else printf("[FAIL] parent unlock error\n");
        close(mtx);
    }
}

int main() {
    debug_mutex(1);

    test_rw();
    test_close_owner();
    test_close_foreign();
    test_exit_with_mutex();
    test_unlock_foreign();
    exit(0);
}
