typedef unsigned int uint;

enum Color { RED, GREEN = 4, BLUE };

static int counter = 0;

int main() {
    uint x = 12;
    enum Color c = BLUE;
    counter += sizeof(x);
    if (c == BLUE) return counter;
    return 0;
}
