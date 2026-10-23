#include <stdio.h>

int main(void) {
    bool enabled = true;
    char quote = '\'';
    char slash = '\\';
    float ratio = 1.5e+2;
    char message[] = "AST " "and TAC\n";

    // Strings may contain characters that look like syntax: { } /* */.
    if (enabled)
        printf("%c%c %.1f %s", quote, slash, ratio, message);
    return 0;
}
