# Cómo funciona LexLP — explicación parte por parte

Esta guía es para sustentar el código: qué hace cada archivo, qué hace
cada función, y cómo señalarlo en pantalla cuando la docente pregunte
"¿dónde haces esto?".

---

## 1. Idea general (di esto primero)

LexLP es un **analizador léxico hecho a mano**: no usa ninguna librería
de expresiones regulares ni generadores como Lex/Flex. Cada expresión
regular de la especificación (`D+`, `L(L|D)*`, etc.) se traduce
directamente a código que recorre el texto **carácter por carácter**.

El flujo es siempre el mismo:

```
archivo .lp  →  main.cpp lo lee completo a un string
             →  Lexer recorre ese string y va devolviendo Tokens
             →  main.cpp imprime/exporta la lista de tokens, la tabla
                de símbolos y los errores
```

Tres archivos, tres responsabilidades:

| Archivo | Responsabilidad |
|---|---|
| `src/Token.h` / `Token.cpp` | qué **es** un token (su estructura) y cómo se imprime |
| `src/Lexer.h` / `Lexer.cpp` | el **autómata**: reconoce los tokens leyendo el texto |
| `src/main.cpp` | lee el archivo, llama al Lexer, imprime/exporta resultados |

---

## 2. `Token.h` / `Token.cpp` — qué es un token

### El enum `TipoToken` (`Token.h`, líneas 16-29)

Es la lista cerrada de todos los tipos de token que el analizador puede
devolver:

```cpp
enum class TipoToken {
    NUM_INT, NUM_DEC, ID, TEXTO,
    INT, FLOAT, CHAR, BOOLEAN, VOID, IF, ELSE, FOR, WHILE, SCANF, PRINTLN, MAIN, RETURN,

    ASIGNACION,                 // =
    SUMA, RESTA, MULT, DIV, MOD, // + - * / %
    AND, OR, NOT,                // && || !
    COMP,                        // > >= < <= != ==  (una sola categoria para las 6)

    PAR_IZQ, PAR_DER,      // ( )
    COR_IZQ, COR_DER,      // [ ]
    LLAVE_IZQ, LLAVE_DER,  // { }
    COMA, PUNTOYCOMA,      // , ;

    ERROR_LEXICO, FIN_ARCHIVO
};
```

- Los primeros 4 corresponden a expresiones regulares (`D+`, `D+\.D+`,
  `L(L|D)*`, `".*"`).
- Los 13 siguientes son las palabras reservadas del lenguaje LP.
- Los operadores y símbolos especiales son "tokens directos": la
  especificación pide que se impriman con su propio símbolo (`<+>`,
  `<;>`, `<&&>`...), no con un nombre. La única excepción es `COMP`:
  las 6 comparaciones (`>`, `>=`, `<`, `<=`, `!=`, `==`) comparten esa
  categoría genérica en vez de tener una cada una — así lo pide la
  especificación.
- `ERROR_LEXICO` no es un token del lenguaje: es la marca interna para
  "este carácter no pertenece al alfabeto de LP".
- `FIN_ARCHIVO` tampoco es un token real: es la señal interna de "ya no
  queda nada por leer", la usa el bucle principal para saber cuándo parar.

### La estructura `Token` (`Token.h`, líneas 39-54)

```cpp
struct Token {
    TipoToken   tipo;
    std::string lexema;    // el texto exacto leído, ej: "edad", "20"
    int         atributo;  // -1, salvo en ID: ahí es el índice en la tabla de símbolos
    int         linea;
    int         columna;
};
```

Cada token que reconoce el Lexer no es solo "qué tipo es", sino también
**exactamente qué texto** se leyó y **dónde empieza** — eso es lo que
permite reportar errores con precisión.

### `nombreTipoToken()` (`Token.cpp`, líneas 3-28)

Un `switch` que traduce cada valor del enum a la cadena que se imprime
en los reportes. Para `NUM_INT`, `ID`, `IF`, etc. devuelve el **nombre**
(`"NUM_INT"`); para los operadores y símbolos especiales devuelve el
**símbolo mismo** (`TipoToken::SUMA` → `"+"`, `TipoToken::PUNTOYCOMA` →
`";"`), porque así lo exige la especificación. `COMP` es la excepción:
devuelve literalmente `"COMP"` sin importar cuál de las 6 comparaciones
fue. Si algún día se agrega un valor al enum y se olvida aquí, el
compilador avisa con `-Wall` (warning de "switch incompleto").

### `Token::aCadena()` (`Token.cpp`, líneas 37-44)

Decide el formato de impresión:

```cpp
if (atributo >= 0) return "<" + nombreTipoToken(tipo) + "," + atributo + ">";
return "<" + nombreTipoToken(tipo) + ">";
```

Por eso un entero se imprime `<NUM_INT>` pero un identificador se
imprime `<ID,0>`, `<ID,1>`... — el número es su posición en la tabla de
símbolos.

---

## 3. `Lexer.h` — el estado del autómata

El Lexer guarda 4 datos mientras recorre el archivo (`Lexer.h`, líneas
40-46):

```cpp
std::string fuente_;   // todo el archivo, cargado en memoria
size_t      pos_;      // en qué posición del string estoy parado
int         linea_;    // línea actual (para reportar errores)
int         columna_;  // columna actual (para reportar errores)

std::vector<Token>       errores_;      // errores acumulados
std::vector<std::string> tablaSimbolos_; // identificadores únicos
```

Se lee **todo el archivo de una vez** (no línea por línea) porque el
automáta necesita *lookahead*: por ejemplo, para saber si un `.` es
parte de un decimal necesita mirar el carácter siguiente aunque esté a
punto de cambiar de línea.

---

## 4. `Lexer.cpp` — método por método

### Utilidades de bajo nivel

- **`esDigito(c)`** (línea 8) — implementa literalmente `D = [0-9]`.
- **`esLetra(c)`** (línea 13) — implementa literalmente `L = [a-zA-Z_]`.
- **`finEntrada()`** (línea 17) — ¿ya se acabó el archivo?
- **`actual()`** (línea 21) — mira el carácter donde estoy parado, sin
  consumirlo.
- **`siguiente()`** (línea 28) — mira **un carácter más adelante**, sin
  consumirlo. Es el *lookahead* que permite decidir si un `.` sigue
  siendo parte de un número decimal.
- **`avanzar()`** (línea 35) — consume el carácter actual, mueve `pos_`
  una posición, y actualiza `linea_`/`columna_`. **Este es el único
  lugar del programa que mueve el cursor por el texto.** Todo lo demás
  llama a `avanzar()`.

### `omitirEspacios()` (línea 47)

Salta espacios, tabs, saltos de línea y `\r` (que aparece si el archivo
se guardó con formato Windows). Se llama al inicio de cada
`siguienteToken()`, antes de intentar reconocer nada.

### `leerNumero()` (línea 69) — implementa `D+` y `D+\.D+`

```cpp
while (esDigito(actual())) lexema += avanzar();      // D+

if (actual() == '.' && esDigito(siguiente())) {       // \. solo si detrás hay D
    lexema += avanzar();                               // consume el punto
    while (esDigito(actual())) lexema += avanzar();    // D+
    return Token(NUM_DEC, ...);
}
return Token(NUM_INT, ...);
```

**El punto de diseño clave (para explicarlo bien):** el `.` solo se
consume si *después* viene al menos un dígito. Por eso:
- `"15.5"` → `NUM_DEC` (el `.` tiene dígito detrás)
- `"15."` → `NUM_INT(15)`, y el `.` queda suelto → se reporta como
  `ERROR_LEXICO` en el siguiente ciclo, porque la ER exige dígitos **a
  ambos lados** del punto.

### `esPalabraReservada()` (línea 93)

Un `std::unordered_map<std::string, TipoToken>` con las 13 palabras
reservadas exactas (`int`, `float`, `if`, `main`, etc.). Si el lexema
coincide **exactamente**, devuelve `true` y el tipo correspondiente. Si
no, devuelve `false` — el lexema es un identificador común.

### `indiceSimbolo()` (línea 117) — la tabla de símbolos

```cpp
for (i = 0; i < tablaSimbolos_.size(); i++)
    if (tablaSimbolos_[i] == lexema) return i;   // ya existe: reusar índice
tablaSimbolos_.push_back(lexema);                 // nuevo: agregar al final
return tablaSimbolos_.size() - 1;
```

Es una lista de identificadores **únicos**, en el orden en que
aparecieron por primera vez. La búsqueda es lineal (no un mapa) a
propósito: la cantidad de identificadores de un programa fuente es
pequeña, así que no hace falta una estructura más compleja — se
mantiene el mismo estilo directo que el resto del autómata.

### `leerIdentificador()` (línea 131) — implementa `L(L|D)*`

```cpp
lexema += avanzar();                                 // primer caracter: L
while (esLetra(actual()) || esDigito(actual()))       // (L|D)*
    lexema += avanzar();

if (esPalabraReservada(lexema, tipo)) return Token(tipo, ...);  // ej: "if" -> IF

indice = indiceSimbolo(lexema);
return Token(ID, lexema, ..., indice);                // ej: "edad" -> ID,0
```

**Punto clave:** primero se lee la palabra **completa**, y recién
después se decide si es reservada o no. Por eso `if2` no colisiona con
`if`: la comparación es exacta, no por prefijo. Solo si NO es reservada
se registra en la tabla de símbolos (las palabras reservadas nunca
entran ahí).

### `leerTexto()` (línea 154) — implementa `".*"`

```cpp
lexema += avanzar();                                  // consume la comilla de apertura
while (actual() != '"' && actual() != '\n')
    lexema += avanzar();

if (actual() == '"') { lexema += avanzar(); return Token(TEXTO, ...); }

// si no se cerró (fin de línea o fin de archivo): error
return Token(ERROR_LEXICO, ...);
```

Si la cadena no se cierra en la misma línea, se reporta como error en
vez de dejar que el problema se arrastre a las líneas siguientes.

### `omitirComentario()` — implementa `//.*\n`

```cpp
avanzar(); avanzar();                          // consume "//"
while (!finEntrada() && actual() != '\n')
    avanzar();
```

Se llama solo cuando ya se confirmó que hay `//` (dos '/' seguidos). No
devuelve ningún `Token`: el comentario se descarta por completo, no
entra a la lista de tokens ni se reporta como error. Por eso vive
*antes* del despachador, no es "un tipo de token más".

### `leerOperadorOSimbolo()` — operadores y símbolos especiales

Reconoce todo lo de las secciones 4.3 y 4.4 de la especificación. Usa
el mismo truco de *lookahead* de un carácter que `leerNumero()` usa
para el punto decimal, pero aquí decide entre un operador de 1 o de 2
caracteres:

```cpp
char c = avanzar();

if (c == '=' && actual() == '=') { avanzar(); return Token(COMP, "=="...); }
if (c == '!' && actual() == '=') { avanzar(); return Token(COMP, "!="...); }
if (c == '<' && actual() == '=') { avanzar(); return Token(COMP, "<="...); }
if (c == '>' && actual() == '=') { avanzar(); return Token(COMP, ">="...); }
if (c == '&' && actual() == '&') { avanzar(); return Token(AND,  "&&"...); }
if (c == '|' && actual() == '|') { avanzar(); return Token(OR,   "||"...); }

switch (c) {
    case '=': return Token(ASIGNACION, "="...);
    case '+': return Token(SUMA, "+"...);
    // ... uno por cada operador/simbolo de 1 caracter
}

// ningun operador/simbolo de LP coincide -> error (ej: '@', '$', '&' suelto)
return Token(ERROR_LEXICO, ...);
```

**Punto clave:** `<`, `>` y `!` por sí solos también son válidos (`<`,
`>` caen en `COMP`; `!` es `NOT`), así que el lookahead solo *amplía*
el operador a su forma de 2 caracteres cuando corresponde — nunca deja
un `<` o un `!` colgando como error si no le sigue `=`.

### `siguienteToken()` — el despachador

Es el corazón del autómata: mira el carácter actual y decide a qué
reconocedor delegar. Antes de eso, alterna entre saltar espacios y
saltar comentarios (pueden venir mezclados: espacio, comentario, más
espacio...) hasta que no quede ninguno de los dos:

```cpp
for (;;) {
    omitirEspacios();
    if (!finEntrada() && actual() == '/' && siguiente() == '/') {
        omitirComentario();
        continue;   // puede haber mas espacios/comentarios despues
    }
    break;
}
if (finEntrada()) return Token(FIN_ARCHIVO, ...);

char c = actual();
if (esDigito(c))  return leerNumero();
if (esLetra(c))   return leerIdentificador();
if (c == '"')     return leerTexto();

return leerOperadorOSimbolo();   // operadores, simbolos, o error si no coincide nada
```

Esto es literalmente la decisión del automáta: **el primer carácter
determina inmediatamente qué expresión regular se está intentando
reconocer.** El caso `//` es especial porque *empieza* igual que el
operador `/` (división): por eso se revisa con `siguiente()` antes de
decidir si es comentario o división — un solo `/` sin otro `/` detrás
cae en `leerOperadorOSimbolo()` y se reconoce como `DIV`.

### `analizarTodo()` (línea 207)

Llama a `siguienteToken()` en bucle hasta `FIN_ARCHIVO`. Los tokens de
tipo `ERROR_LEXICO` **no** entran a la lista de tokens válidos — se
descartan ahí mismo porque ya quedaron guardados en `errores_` dentro
del propio reconocedor que los generó. Así el análisis nunca se
detiene en el primer error: sigue leyendo y reporta *todos* los errores
del archivo en una sola pasada.

---

## 5. `main.cpp` — el punto de entrada

1. **`leerArchivo()`** (línea 27): abre el `.lp` y lo vuelca completo a
   un `std::string` usando un `stringstream`.
2. **`main()`** (línea 97):
   - crea el `Lexer` con el contenido del archivo,
   - llama a `lexer.analizarTodo()`,
   - imprime en consola: lista de tokens, detalle línea/columna, tabla
     de símbolos, errores y totales,
   - exporta lo mismo a `output/tokens.txt` y `output/errores.txt`.
3. Las funciones `imprimirListaTokens`, `imprimirDetalleTokens`,
   `imprimirTablaSimbolos`, `imprimirErrores` (líneas 40, 56, 70, 82)
   solo dan formato a lo que el Lexer ya calculó — no hacen ningún
   análisis léxico, son puramente de presentación.

---

## 6. Trazado de un ejemplo (para explicarlo en vivo)

Con `int edad` como entrada:

| Paso | `pos_` apunta a | Qué pasa |
|---|---|---|
| 1 | `i` | `siguienteToken()` ve una letra → llama a `leerIdentificador()` |
| 2 | — | consume `i n t`, se detiene en el espacio (no es letra ni dígito) |
| 3 | — | `esPalabraReservada("int", ...)` → `true` → devuelve `Token(INT, "int")` |
| 4 | (espacio) | `omitirEspacios()` lo salta |
| 5 | `e` | letra de nuevo → `leerIdentificador()` |
| 6 | — | consume `e d a d` |
| 7 | — | `esPalabraReservada("edad", ...)` → `false` |
| 8 | — | `indiceSimbolo("edad")` → no existe, lo agrega, devuelve `0` |
| 9 | — | devuelve `Token(ID, "edad", atributo=0)` → se imprime `<ID,0>` |

Salida: `<INT> <ID,0>`.

---

## 7. "Cámbiame esto y compílalo" — guía rápida para pedidos en vivo

| Pedido | Qué tocar | Ejemplo |
|---|---|---|
| Agregar una palabra reservada nueva | `Lexer.cpp`, mapa dentro de `esPalabraReservada()` (línea ~94) **y** el enum en `Token.h` (línea 23-25) **y** el `switch` en `Token.cpp` (línea 10-22) | agregar `"break"` → `TipoToken::BREAK` |
| Agregar un operador/símbolo nuevo | el enum en `Token.h` **y** el `switch` en `Token.cpp` (nombreTipoToken) **y** el `switch` de `leerOperadorOSimbolo()` en `Lexer.cpp` | agregar `^` → `TipoToken::POTENCIA` |
| Cambiar cómo se imprime un token | `Token.cpp`, función `nombreTipoToken()` | que `ID` se imprima como `IDENTIFICADOR` |
| Cambiar el archivo de entrada por defecto | `main.cpp`, línea 98 | usar `tests/prueba_basica.lp` |
| Probar con un archivo nuevo sin tocar código | no se edita nada | `LexLP.exe tests\loquesea.lp` |
| Que un identificador pueda empezar distinto | `Lexer.cpp`, función `esLetra()` (línea 13) | agregar algún carácter al alfabeto `L` |

**Procedimiento siempre igual:** editar → guardar → `compilar.bat` →
`LexLP.exe <archivo>`. Si agregas un valor al enum `TipoToken` y se te
olvida agregarlo también en el `switch` de `nombreTipoToken()`, el
compilador te lo va a advertir (por el `-Wall`) — es una forma de
"red de seguridad" que puedes mencionar si preguntan por qué se
compila con esas banderas.
