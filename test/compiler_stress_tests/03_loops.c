int main() {
    int sum = 0;
    
    for (int i = 0; i < 5; i = i + 1) {
        sum = sum + i;
    }
    
    int j = 5;
    while (j > 0) {
        sum = sum * 2;
        j = j - 1;
    }
    
    int k = 0;
    do {
        sum = sum - 1;
        k = k + 1;
    } while (k < 3);

    return 0;
}