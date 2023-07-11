// You may add up to 3 elements to this struct.
// The type of synchronization primitive you may use is SpinLock.
typedef struct biscuit_juice_lock {
    bool locked = 0;
    int juice = 0;
    int biscuit = 0;

}fd_lock_t;

void produce_biscuit(fd_lock_t* fd) {
    fd.biscuit += 1;
    fd.locked = 1;
}

void consume_biscuit(fd_lock_t* fd) {
    while (fd.locked == 0){
        fd.locked = fd.locked;
    }
    if (fd.juice != 0){
        consume_juice(fd);
        return;
    }
    if (fd.biscuit > 0){
        fd.biscuit -= 1;
    }
    fd.locked = 0;
}

int produce_juice(fd_lock_t* fd) {
    if (fd.juice < 4){
        fd.juice += 1;
        fd.locked = 1;
        return 0;
    }
    fd.locked = 1;
    return -1;
    
}

void consume_juice(fd_lock_t* fd) {
    while (fd.locked == 0){
        fd.locked = fd.locked;
    }
    if (fd.juice != 0){
        fd.juice -= 1;
    }
    fd.locked = 0;
    return;
}