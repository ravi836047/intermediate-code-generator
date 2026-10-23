class Node {
public:
    int data;
    Node* next;
    
    void init(int v) {
        data = v;
        next = 0; // using 0 for nullptr
    }
};

int main() {
    Node* head = new Node;
    head->init(1);
    
    Node* current = head;
    
    for (int i = 2; i <= 5; i = i + 1) {
        current->next = new Node;
        current = current->next;
        current->init(i);
    }
    
    int sum = 0;
    current = head;
    while (current != 0) {
        sum = sum + current->data;
        Node* temp = current;
        current = current->next;
        delete temp;
    }
    
    return sum;
}