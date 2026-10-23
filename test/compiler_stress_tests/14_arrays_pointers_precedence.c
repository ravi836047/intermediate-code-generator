int main() {
    int a[3] = {10, 20, 30};
    int *p = a;
    *(p + 1) = a[0] + a[2];
    return *(p + 1);
}
