int fact(int n) {
    if (n <= 1) return 1;
    return n * fact(n - 1);
}

int combine(int x) {
    return x + 1;
}

int combine(int x, int y) {
    return x * y;
}

int main() {
    int a = fact(5);
    int b = combine(a);
    int c = combine(3, 4);
    return b + c;
}
