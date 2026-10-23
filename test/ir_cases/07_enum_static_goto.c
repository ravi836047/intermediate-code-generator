enum State { READY, RUNNING = 3, STOPPED, };

static int bump(int value) {
    return value + 1;
}

int main(void) {
    int count = 0;

retry:
    count = bump(count);
    if (count < STOPPED)
        goto retry;
    return count;
}
