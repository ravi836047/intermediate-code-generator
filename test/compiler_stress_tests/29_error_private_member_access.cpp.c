class Vault {
private:
    int secret;
public:
    Vault() { this->secret = 9; }
};
int main() {
    Vault v;
    v.secret = 2;
    return 0;
}
