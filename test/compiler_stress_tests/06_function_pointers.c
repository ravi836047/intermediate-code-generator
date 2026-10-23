int add(int a, int b) {
    return a + b;
}

int main() {
    int (*op)(int, int);
    op = add;
    return op(2, 3);
}
