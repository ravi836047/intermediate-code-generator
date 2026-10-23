int add(int x, int y) {
    return x + y;
}

int apply(int (*fn)(int, int), int x, int y) {
    return fn(x, y);
}

int main(void) {
    int count = 3;
    int *data = new int[count];

    data[0] = 4;
    data[1] = 7;
    int total = apply(add, data[0], data[1]);
    delete[] data;
    return total;
}
