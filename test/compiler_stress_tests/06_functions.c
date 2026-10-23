int add(int a, int b) {
    return a + b;
}

int multiply(int x, int y) {
    return x * y;
}

int main() {
    int sum = add(5, 7);
    int product = multiply(sum, 2);
    return product;
}