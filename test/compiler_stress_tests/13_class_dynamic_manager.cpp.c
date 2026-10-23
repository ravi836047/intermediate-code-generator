class ResourceManager {
private:
    int* buffer;
    int size;

public:
    void allocate(int s) {
        size = s;
        buffer = new int[size];
    }

    void fill(int val) {
        for (int i = 0; i < size; i = i + 1) {
            buffer[i] = val + i;
        }
    }

    int get(int index) {
        return buffer[index];
    }

    void cleanup() {
        delete[] buffer;
    }
};

int main() {
    ResourceManager manager;
    manager.allocate(5);
    manager.fill(10);
    
    int val = manager.get(3); // Should retrieve 13
    manager.cleanup();
    
    return val;
}