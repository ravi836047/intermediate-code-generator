int main(void) {
    int a = 5, b = 3, c = 0;

    c = a + b * 4 - (a << 1);
    c += a % b;
    c = (c >= 0 && a != b) || !b;
    return c;
}
