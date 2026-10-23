// Tests max-munch tokenization, pointer ambiguities, and nested ternaries.
int main() {
    int a = 10, b = 5, c = 2;
    int *p = &a;
    
    // a * **(&p) -> pointer depth vs multiplication
    int val = a***(&p); 
    
    // Max munch: a-- - b
    int x = a---b;      
    
    // Nested ternary madness
    int y = (a > b) ? (b > c) ? b : c : val ? a : b;
    
    // Concatenated string literals with embedded comment traps
    char str[] = "this is " "a string" " with \"escapes\" and /* no comments */";
    
    return y;
}