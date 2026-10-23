class Base {
public:
    int x;
    Base(int v) { this->x = v; }
};

class Derived : public Base {
public:
    int y;
    Derived(int a, int b) : Base(a) { this->y = b; }
    int sum() { return this->x + this->y; }
};

int main() {
    Derived d(2, 5);
    return d.sum();
}
