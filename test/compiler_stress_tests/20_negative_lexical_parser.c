// Lexer/Parser stress test: Malformed tokens and grammar traps.
int main() {
    int a = 0xG;       // Invalid hex
    char c = '';       // Empty char
    float f = .5.6;    // Bad float
    
    int b = 10;
    b = b + @;         // Invalid char
    
    int goto = 5;      // Keyword as identifier
    
    // Unterminated block comment
    /* 
    return 0;
}