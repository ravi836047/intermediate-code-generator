enum Status { READY, RUNNING = 3, DONE, };

static int step(int value) {
    return value + 1;
}

int main() {
    int count = 0;
start:
    count = step(count);
    if (count < DONE) goto start;
    return count;
}
