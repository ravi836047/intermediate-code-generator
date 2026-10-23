int main() {
    int a = 7;
    float b = 2.5;
    char c = 'x';
    bool ok = true;
    int d = (a + 3) * 2 - 4 / 2;
    float e = b + 1.5 * 2.0;
    a += 5;
    a--;
    ++a;
    if (ok && a >= 10 && c != 'z') return d;
    return 0;
}
