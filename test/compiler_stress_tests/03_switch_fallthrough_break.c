int main() {
    int x = 2;
    int y = 0;
    switch (x) {
        case 1: y = 10; break;
        case 2:
            y = 20;
            y += 1;
            break;
        case 3: y = 30; break;
        default: y = -1;
    }
    return y;
}
