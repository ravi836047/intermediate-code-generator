int main() {
    int n = 3;
    int sum = 0;
start:
    if (n <= 0) goto done;
    sum += n;
    n--;
    goto start;
done:
    return sum;
}
