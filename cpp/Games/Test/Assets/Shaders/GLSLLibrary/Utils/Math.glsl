#define EPS 1e-5
#define PI 3.141592653589

bool FloatEqual(float a, float b) {
    return abs(a - b) < EPS;
}