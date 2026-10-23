struct Point {
    int x;
    int y;
};

int main() {
    struct Point p;
    p.x = 15;
    p.y = 25;
    
    int distance_sq = (p.x * p.x) + (p.y * p.y);
    return 0;
}