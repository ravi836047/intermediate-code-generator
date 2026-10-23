int main() {
    int x = 10;
    int *p = &x;
    int **pp = &p;
    int ***ppp = &pp;
    **pp = 12;
    ***ppp = ***ppp + 1;
    return x;
}
