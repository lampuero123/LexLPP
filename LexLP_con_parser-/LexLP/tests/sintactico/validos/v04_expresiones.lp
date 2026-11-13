void main() {
    int a, b;
    boolean ok;
    a = 2 + 3 * 4 - 10 / 5 % 2;
    b = (a + 1) * -a + +3;
    ok = !(a > b) || a == b && b != 0;
    ok = !!ok;
    return a;
}
