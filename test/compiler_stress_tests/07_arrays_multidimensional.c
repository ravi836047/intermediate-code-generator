int main() {
    int a[4] = {1, 2, 3, 4};
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    a[1] = a[0] + a[3];
    matrix[1][2] = a[1];
    return matrix[1][2];
}
