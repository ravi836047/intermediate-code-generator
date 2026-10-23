class Calculator {
private:
    int memory;

public:
    void setMemory(int val) {
        memory = val;
    }

    int add(int a, int b) {
        return a + b + memory;
    }
};

int main() {
    Calculator calc;
    calc.setMemory(10);
    int total = calc.add(5, 5);
    return 0;
}