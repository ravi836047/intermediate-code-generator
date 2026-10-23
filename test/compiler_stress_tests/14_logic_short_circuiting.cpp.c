int counter = 0;

int increment() {
    counter = counter + 1;
    return counter;
}

int main() {
    bool a = false;
    bool b = true;
    
    // Short circuit: increment() should NOT be called
    if (a && (increment() > 0)) {
        counter = 100;
    }
    
    // Short circuit: increment() should NOT be called
    if (b || (increment() > 0)) {
        counter = counter + 10;
    }
    
    // Will be called
    if (!a && (increment() > 0)) {
        counter = counter + 1000;
    }
    
    return counter; // 0 + 10 + 1000 + 1 = 1011
}