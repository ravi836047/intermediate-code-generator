int main() {
    int* d_ptr = new int;
    *d_ptr = 42;
    
    int val = *d_ptr;
    
    delete d_ptr;
    
    int* d_arr = new int[5];
    d_arr[0] = 100;
    delete[] d_arr;

    return 0;
}