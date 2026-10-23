int global_var = 100;

int main() {
    int x = 1;
    if (global_var == 100) {
        int x = 2; // Shadows outer x
        for (int i = 0; i < 5; i = i + 1) {
            int x = 3; // Shadows if-block x
            global_var = global_var + x;
        }
    }
    return x; // Should return 1, not 2 or 3
}