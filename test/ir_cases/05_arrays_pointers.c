int main(void) {
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    char word[] = "ok";
    int *row = matrix[1];
    int **alias = &row;

    matrix[0][2] = *row + **alias;
    return matrix[0][2] + word[0];
}
