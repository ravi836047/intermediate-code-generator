struct Pair {
    int left;
    int right;
};

class Vault {
public:
    int visible;
private:
    int secret;
protected:
    int inherited;
};

int main(void) {
    struct Pair pair;
    class Vault vault;

    pair.left = 4;
    pair.right = 9;
    vault.visible = pair.left + pair.right;
    return vault.visible;
}
