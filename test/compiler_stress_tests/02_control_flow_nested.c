int main() {
    int i = 0;
    int j = 0;
    int total = 0;
    for (i = 0; i < 5; i++) {
        if (i == 2) continue;
        j = 0;
        while (j < 4) {
            j++;
            if (j == 3) break;
            total += i + j;
        }
    }
    do {
        total--;
    } while (total > 20);
    return total;
}
