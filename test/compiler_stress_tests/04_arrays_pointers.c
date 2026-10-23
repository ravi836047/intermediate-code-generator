int main() {
    int arr[3];
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;

    int* ptr = &arr[0];
    int val1 = *ptr;
    
    ptr = ptr + 1; 
    int val2 = *ptr; 

    return 0;
}