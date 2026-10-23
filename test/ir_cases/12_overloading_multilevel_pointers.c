int blend(int value) {
    return value + 1;
}

int blend(int left, int right) {
    return left * right;
}

int main(void) {
    int value = 3;
    int *level1 = &value;
    int **level2 = &level1;
    int ***level3 = &level2;

    return blend(***level3) + blend(**level2, 5);
}
