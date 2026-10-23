typedef int Count;

class Parent {
public:
    int inherited;
};

class Child : public Parent {
public:
    int own;

    int total() {
        return this->inherited + this->own;
    }
};

int main(void) {
    Count value = 4;
    Count &alias = value;
    Count *pointer = &alias;
    Count **chain = &pointer;
    int bytes = sizeof(int) + sizeof(value) + sizeof *pointer;

    return **chain + bytes;
}
