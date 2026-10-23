int main() {
    int a = 4;
    int b = 9;
    int c = (a < b) ? ((b > 5) ? b : a) : 0;
    if (!(a == 0) || (b <= 10 && c != 0)) {
        return c;
    }
    return -1;
}
