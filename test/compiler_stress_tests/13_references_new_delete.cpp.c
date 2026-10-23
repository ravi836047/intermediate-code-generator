int main() {
    int x = 5;
    int &r = x;
    r = r + 2;
    int *p = new int(9);
    int value = *p;
    delete p;
    return x + value;
}
