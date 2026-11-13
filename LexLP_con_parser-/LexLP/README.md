# LexLP — Analizador Léxico para el lenguaje LP

**Curso:** Compiladores
**Docente:** Prof. Nury Y. Arosquipa Yanque
**Lenguaje de implementación:** C++17

Analizador léxico que lee un programa fuente escrito en LP y lo transforma en
una secuencia de tokens, reportando además los errores léxicos con su ubicación
exacta (línea y columna).

El autómata está implementado **a mano**: cada expresión regular de la
especificación se traduce directamente a código que recorre el texto carácter
por carácter. No se utiliza ninguna librería de expresiones regulares ni
generadores tipo Lex/Flex.

---

## Estado por fases

| Entrega | Contenido | Estado |
|---|---|---|
| Semana 1 (15/09) | Lectura del archivo + `NUM_INT` + `NUM_DEC` |  Implementado |
| Semana 2 (22/09) | `ID` + `TEXTO` + palabras reservadas + tabla de símbolos | Implementado |
| Semana 3 (29/09) | Operadores + símbolos especiales + comentarios + integración completa |  Implementado |

### Expresiones regulares implementadas

Variables base: `D = [0-9]` y `L = [a-zA-Z_]`.

| ER | Token | Ejemplo |
|---|---|---|
| `D+` | `NUM_INT` | `20` |
| `D+\.D+` | `NUM_DEC` | `15.5` |
| `L(L\|D)*` | `ID` | `edad` |
| `".*"` | `TEXTO` | `"hola"` |
| `int, float, char, boolean, void, if, else, for, while, scanf, println, main, return` | `INT, FLOAT, CHAR, BOOLEAN, VOID, IF, ELSE, FOR, WHILE, SCANF, PRINTLN, MAIN, RETURN` | `if` |

Una palabra reservada tiene prioridad sobre `ID`: `if` produce `<IF>`, pero
`if2` sigue siendo un `ID` porque no coincide exactamente con la palabra
reservada.

### Operadores, símbolos especiales y comentarios (Semana 3)

| Lexema | Categoría | Token impreso |
|---|---|---|
| `=` | Asignación | `<=>` |
| `+` `-` `*` `/` `%` | Aritméticos | `<+>` `<->` `<*>` `</>` `<%>` |
| `&&` `\|\|` `!` | Lógicos | `<&&>` `<\|\|>` `<!>` |
| `>` `>=` `<` `<=` `!=` `==` | Relacionales | `<COMP>` (categoría única para las 6, según especificación) |
| `(` `)` `[` `]` `{` `}` `,` `;` | Símbolos especiales | `<(>` `<)>` `<[>` `<]>` `<{>` `<}>` `<,>` `<;>` |
| `//.*\n` | Comentario de línea | se descarta: no genera token ni error |

`/` se distingue de un comentario mirando el carácter siguiente: si también
es `/`, todo lo que sigue hasta el fin de línea se descarta; si no, `/` es el
operador de división.

Cualquier carácter (o cadena de texto sin cerrar) que no encaje en ninguna
de las reglas anteriores se reporta como `ERROR_LEXICO`, con su línea y
columna exactas.

---

## Estructura del proyecto

```
LexLP/
├── src/
│   ├── main.cpp        # lectura del archivo y reportes de salida
│   ├── Lexer.h/.cpp    # el autómata: recorre el fuente y reconoce tokens
│   └── Token.h/.cpp    # estructura Token y nombres de los tipos
├── tests/
│   ├── prueba_semana1.lp          # números enteros y decimales (fase actual)
│   ├── prueba_semana1_limites.lp  # casos límite: 15.  .5  3..14  @
│   ├── prueba_basica.lp                    # variables y tabla de símbolos (Semana 2)
│   ├── prueba_identificadores.lp           # ID vs. palabras reservadas (if2, Int, IF, _privado...)
│   ├── prueba_identificadores_repetidos.lp # un ID repetido reutiliza su índice en la tabla
│   ├── prueba_texto_casos.lp               # TEXTO: normal, vacío y sin cerrar
│   ├── prueba_operadores_aritmeticos.lp    # + - * / %
│   ├── prueba_comparaciones.lp             # >= != == (categoría COMP)
│   ├── prueba_logicos.lp                   # && y !
│   ├── prueba_comentarios.lp               # // se descarta, no genera token
│   ├── prueba_completa.lp                  # programa completo en LP (Semana 3)
│   ├── prueba_errores.lp                   # errores léxicos (@, $)
│   ├── expected/                           # salida esperada de cada prueba (para run_tests.sh)
│   └── run_tests.sh               # corre todas las pruebas y compara contra expected/
├── output/
│   ├── tokens.txt      # generado al ejecutar
│   └── errores.txt     # generado al ejecutar
├── compilar.bat
└── README.md
```

---

## Compilación y ejecución

```bat
compilar.bat
LexLP.exe tests\prueba_semana1.lp
```

### Funcionalidades adicionales

- **Coloreado sintáctico en consola**: cada categoría de token se imprime en
  un color distinto (números, identificadores, texto, palabras reservadas,
  errores en rojo). Los archivos exportados a `output/` quedan siempre en
  texto plano, sin códigos de color.

- **Visualización paso a paso**: `LexLP.exe <archivo.lp> --visualizar`
  recorre el archivo token por token, resaltando en el texto fuente
  completo cuál es el lexema que se acaba de reconocer, y espera Enter
  antes de continuar con el siguiente. Útil para explicar el
  funcionamiento del autómata en vivo.

- **Pruebas automatizadas**: `tests/run_tests.sh` compila el proyecto y
  corre todos los `.lp` de `tests/` que tengan un resultado guardado en
  `tests/expected/`, comparándolo con `diff`. Si un cambio de
  comportamiento es intencional, `tests/run_tests.sh --actualizar`
  regenera esa referencia.

O de forma manual:

```bat
g++ -std=c++17 -Wall -o LexLP.exe src\main.cpp src\Lexer.cpp src\Token.cpp
```

Si no se indica archivo, se usa `tests/prueba_semana1.lp` por defecto.

---

## Decisiones de diseño

- **Lectura completa del archivo en memoria.** El analizador necesita poder
  mirar el carácter siguiente (*lookahead*) aunque se encuentre al final de una
  línea; leer línea por línea rompería esa capacidad.

- **Lookahead de un carácter para el punto decimal.** El punto solo se consume
  si después viene al menos un dígito. Por eso `15.5` produce `NUM_DEC`,
  mientras que `15.` produce `NUM_INT(15)` y el punto queda suelto para
  reportarse como error. Esto respeta la ER `D+\.D+`, que exige dígitos a
  ambos lados del punto.

- **Línea y columna se actualizan al consumir cada carácter**, dentro de
  `avanzar()`. Así todo token conoce su ubicación de inicio sin necesidad de
  recorrer el texto una segunda vez.

- **Los errores no entran en la lista de tokens.** Se acumulan en una lista
  aparte, de modo que el análisis continúa y se reportan *todos* los errores
  del archivo en una sola ejecución, en lugar de detenerse en el primero.

- **El campo `atributo` del token** vale `-1` para todo token que no sea
  `ID`. Para un `ID` guarda su posición en la tabla de símbolos, y la
  impresión pasa automáticamente de `<TIPO>` a `<ID,0>` sin necesidad de
  tocar `Token::aCadena()`.

- **Tabla de símbolos como lista de identificadores únicos, en orden de
  aparición.** Al leer un `ID` se busca linealmente en la tabla: si ya
  existe se reutiliza su índice, si no, se agrega al final. Ese índice es
  el `atributo` del token. Es una búsqueda lineal (no un `map`) porque el
  volumen de identificadores de un programa fuente es pequeño y así el
  código queda igual de directo que el resto del autómata.

- **Palabras reservadas se resuelven después de reconocer el `ID` completo.**
  El lexema se compara contra una tabla fija (`int`, `if`, `main`, ...); si
  coincide exactamente se devuelve el token reservado (`<IF>`, `<MAIN>`,
  ...) y NO se registra en la tabla de símbolos. Por eso `if2` sigue siendo
  un `ID` normal: la coincidencia es exacta, no por prefijo.

- **Una cadena de texto (`".*"`) sin comilla de cierre en la misma línea**
  se reporta como `ERROR_LEXICO` (con el texto leído hasta el corte), en
  vez de dejar que el analizador arrastre el error a las líneas
  siguientes.

---

# Analizador Sintáctico (Proyecto integrador — Entregable 1)

Parser de **descenso recursivo** para la gramática LL(1) de LP. Consume los
tokens que produce el lexer; nunca vuelve a leer caracteres del fuente.

```
archivo.lp → Lexer → tokens → Parser → PROGRAMA ACEPTADO / RECHAZADO (+ mensaje)
```

## Compilar y ejecutar

```
compilar.bat                                   (Windows: genera LexLP.exe y ParserLP.exe)
ParserLP.exe tests\sintactico\validos\v01_minimo.lp
```

En Linux/WSL:

```
g++ -std=c++17 -Wall -Wextra -o ParserLP src/mainParser.cpp src/Parser.cpp src/Lexer.cpp src/Token.cpp
./ParserLP tests/sintactico/validos/v01_minimo.lp
```

Código de salida: `0` aceptado, `1` rechazado, `2` archivo no encontrado.
El informe también se guarda en `output/sintactico.txt`.

## Módulos

| Archivo | Responsabilidad |
| --- | --- |
| `src/Lexer.h/.cpp`, `src/Token.h/.cpp` | Analizador léxico. Se agregó el token `FUNC` (palabra reservada `func`) y `analizarConFin()`, que entrega los tokens terminados en `FIN_ARCHIVO`. |
| `src/Parser.h/.cpp` | Parser: una función por cada no terminal (`programa()`, `bloque()`, `listaVariables()`, `asignacion()`, `expresionAditiva()`, …). |
| `src/mainParser.cpp` | Integración: lee el `.lp`, llama al lexer, luego al parser, e imprime el resultado. |

## Cómo funciona el parser

- `esperar(tipo)` consume el token si coincide; si no, lanza `ErrorSintactico`.
- Las producciones con ε se resuelven con `if`/`while` que consultan el conjunto
  PRIMERO (ej. `esInicioComando()`).
- Un nivel de expresión por función: `||` → `&&` → comparación → `+ -` → `* / %` → factor.
  Por eso `*` agrupa antes que `+` y los operadores son asociativos a la izquierda.
- Mensaje de error: ubicación, qué se esperaba y qué se encontró, con una pista
  cuando es un olvido típico (`;` al final de la línea anterior, declaración
  después de un comando).
- Si hay errores léxicos no se ejecuta el parser (los tokens no son confiables).
- En esta entrega el parser se detiene en el **primer** error sintáctico.

## Decisiones respecto a la gramática

- **`void main()`**: la gramática dice `PRINCIPAL → main ( ) BLOQUE`, pero el
  ejemplo oficial usa `void main()`. Se acepta un `void` opcional antes de `main`.
- **`func`** se agregó al lexer como palabra reservada.
- **`for`** sigue en el lexer, pero la gramática no lo incluye: el parser lo rechaza.
- **`return`** exige expresión (`return;` es error).
- **Llamada como comando** (`func saludar()`) no lleva `;`, tal como define la gramática;
  dentro de una asignación el `;` lo pone `ASIGNACION`.
- **Arreglos**: la dimensión va en el tipo (`int[10] v;`), no en el nombre. La
  gramática tampoco permite asignar a un elemento (`v[0] = 1;` es inválido) ni
  `-(expr)` (el signo solo precede a un id o una constante).

## Pruebas

```
tests/run_tests_parser.sh        (-v para ver los mensajes de error)
```

- `tests/sintactico/validos/`: 4 programas que deben ser aceptados
  (mínimo, integral, funciones/arreglos/lectura, expresiones).
- `tests/sintactico/invalidos/`: 8 casos que deben ser rechazados (`;` faltante,
  llave sin cerrar, paréntesis del `if`, expresión incompleta, declaración tardía,
  `return;`, error léxico, falta `main`).
