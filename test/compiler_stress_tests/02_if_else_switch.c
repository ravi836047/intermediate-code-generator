int main() {
    int val = 2;
    int result = 0;

    if (val > 5) {
        result = 10;
    } else if (val == 2) {
        result = 20;
    } else {
        result = 30;
    }

    switch (result) {
        case 10:
            val = 100;
            break;
        case 20:
            val = 200;
            break;
        default:
            val = 404;
    }
    return 0;
}