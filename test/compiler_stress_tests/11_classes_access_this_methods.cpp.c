class Box {
public:
    int value;

    Box(int v) {
        this->value = v;
    }

    int get() {
        return this->value;
    }

private:
    int secret;
};

int main() {
    Box b(7);
    b.value = b.get();
    return b.value;
}
