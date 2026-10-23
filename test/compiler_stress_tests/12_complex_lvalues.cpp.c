struct Point {
    int x;
    int y;
};

struct Polygon {
    Point vertices[10];
};

int main() {
    Polygon poly;
    
    for (int i = 0; i < 10; i = i + 1) {
        poly.vertices[i].x = i * 2;
        poly.vertices[i].y = i * 3;
    }
    
    int result = poly.vertices[4].x + poly.vertices[4].y; 
    return result;
}