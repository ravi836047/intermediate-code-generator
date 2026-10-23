int classify(int value) {
    switch (value) {
        case 0:
            return 10;
        case 1:
        case 2:
            return 20;
        default:
            return -1;
    }
}

int main(void) {
    return classify(2);
}
