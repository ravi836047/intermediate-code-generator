// Tests IR label management, nested scopes, and scope-breaking jumps.
int main() {
    int sum = 0;
    for (int i = 0; i < 10; i = i + 1) {
        if (i % 2 == 0) continue;
        
        switch (i) {
            case 3: 
                sum = sum + 10; 
                break; // Breaks switch, not loop
            case 5: 
                goto skip; // Jumps completely out of loop
            case 7:
                do {
                    sum = sum + 1;
                    if (sum > 20) break; // Breaks do-while
                } while (true);
                continue; // Continues for-loop
            default:
                sum = sum + i;
        }
    }
skip:
    return sum;
}