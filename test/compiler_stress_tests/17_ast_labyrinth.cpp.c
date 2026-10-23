// Tests deep L-Value resolution, pointer arithmetic, and struct arrays.
struct Inner { 
    int val; 
};

struct Outer { 
    Inner* in_arr[5]; 
};

int main() {
    Outer out;
    Inner in_obj;
    in_obj.val = 42;
    
    out.in_arr[3] = &in_obj;
    
    // Deep L-Value assignment
    out.in_arr[3]->val = out.in_arr[3]->val + 10;
    
    // Pointer arithmetic combined with member access
    Inner** ptr = &out.in_arr[2];
    (*(ptr + 1))->val = 100; // sets in_arr[3]->val
    
    return out.in_arr[3]->val;
}