int main(void) {
    int i = 0, total = 0;

    while (i < 6) {
        i++;
        if (i % 2 == 0)
            continue;
        total += i;
    }

    do {
        total += i;
        i--;
    } while (i > 3);

    for (int j = 0; j < 3; j++) {
        if (j == 2)
            break;
        total += j;
    }

    return total;
}
