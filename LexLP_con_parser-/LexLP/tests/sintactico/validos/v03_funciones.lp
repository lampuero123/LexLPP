// funciones antes de main, parametros, llamadas y arreglos
int suma(int a, int b) {
    int r;
    r = a + b;
    return r;
}

int[5] vacio() {
    int[5] v;
    return 0;
}

void saludar() {
    println("hola");
}

main() {
    int x, z;
    int[10] y;
    int[3][4] m;
    int total;
    scanf(x, y[0], m[1][2]);
    total = func suma(x, y[0] * 2);
    func saludar()
    z = y[x + 1] + m[1][2];
    println("Total:", total, 3.14);
    return total;
}
