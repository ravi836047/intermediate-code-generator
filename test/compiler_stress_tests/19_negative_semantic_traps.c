// Semantic stress test: Valid syntax, but illegal operations.
class B { 
private: 
    int y; 
}; 

int main() {
    // Trap 1: Array direct assignment (Illegal in C/C++)
    int arr1[3];
    int arr2[3];
    arr2 = arr1; 
    
    // Trap 2: Invalid pointer cast
    int x = 5;
    int* p = x; 
    
    // Trap 3: Private member access
    B obj;
    obj.y = 10; 
    
    // Trap 4: Break outside loop/switch
    if (x == 5) { 
        break; 
    } 
    
    return 0;
}