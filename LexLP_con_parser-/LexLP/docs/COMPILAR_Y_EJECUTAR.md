# Cómo compilar y ejecutar LexLP

Guía rápida para la sustentación: cómo compilar, cómo correr un archivo
`.lp`, y qué hacer si la docente pide cambiar algo en el momento y volver
a compilar.

---

## 1. Requisitos

- Un compilador de C++ con soporte para C++17: **g++** (MinGW en Windows,
  o el g++ de Linux/WSL).
- No se necesita ninguna librería externa. Todo el proyecto usa solo la
  biblioteca estándar de C++ (`<string>`, `<vector>`, `<fstream>`,
  `<unordered_map>`, `<filesystem>`).

Para comprobar que tienes g++ instalado:

```bat
g++ --version
```

Si da error "no se reconoce como comando", instala MinGW-w64 y agrega su
carpeta `bin` al PATH (o usa la ruta completa, ver `compilar.bat`).

---

## 2. Compilar

### Opción A: con el script (la forma normal de entregar)

```bat
compilar.bat
```

Esto ejecuta por dentro:

```bat
g++ -std=c++17 -Wall -Wextra -static -o LexLP.exe src\main.cpp src\Lexer.cpp src\Token.cpp
```

Y genera `LexLP.exe` en la raíz del proyecto.

**Qué significa cada parte del comando**, por si la docente pregunta:

| Parte | Significado |
|---|---|
| `-std=c++17` | usa el estándar C++17 del lenguaje |
| `-Wall -Wextra` | activa advertencias del compilador (ayuda a detectar errores de lógica, no solo de sintaxis) |
| `-static` | el `.exe` queda autocontenido, corre en cualquier Windows sin instalar nada más |
| `-o LexLP.exe` | nombre del ejecutable de salida |
| `src\main.cpp src\Lexer.cpp src\Token.cpp` | los tres archivos `.cpp` que se compilan y se enlazan juntos |

### Opción B: comando manual (si el script falla o estás en Linux/WSL)

```bash
g++ -std=c++17 -Wall -Wextra -o LexLP.exe src/main.cpp src/Lexer.cpp src/Token.cpp
```

(quitando `-static`, que en Linux no aplica igual).

---

## 3. Ejecutar

```bat
LexLP.exe tests\prueba_basica.lp
```

- Si no se indica ningún archivo, usa por defecto `tests/prueba_semana1.lp`
  (ver `main.cpp`, línea 98).
- Puedes pasarle cualquier archivo `.lp`, incluso uno que la docente te
  pida crear ahí mismo.

### Qué muestra en pantalla

1. **Lista de tokens** agrupada por línea del programa fuente, en el
   formato `<TIPO>` o `<ID,n>` cuando aplica.
2. **Detalle** con línea/columna de cada token — útil para señalar
   exactamente dónde se reconoció algo.
3. **Tabla de símbolos**: identificadores únicos, en el orden en que
   aparecieron, con su índice.
4. **Errores léxicos**: cada carácter que no encajó en ninguna regla,
   con su línea y columna.
5. **Totales**: cantidad de tokens, identificadores y errores.

### Qué genera como archivo

- `output/tokens.txt` — lista de tokens + detalle + tabla de símbolos.
- `output/errores.txt` — reporte de errores léxicos.

(Estos archivos se sobrescriben cada vez que corres el programa; por
eso no están en git, ver `.gitignore`.)

---

## 4. Funcionalidades adicionales

### Coloreado en consola

La salida por consola ya viene coloreada por categoría de token (números,
identificadores, texto, palabras reservadas, errores en rojo). No requiere
nada extra: es automático al ejecutar `LexLP.exe`. Los archivos exportados
en `output/` no llevan color, quedan en texto plano.

### Visualización paso a paso

```bat
LexLP.exe tests\prueba_basica.lp --visualizar
```

Muestra el texto fuente completo en cada paso, resaltando el lexema que
se acaba de reconocer, y espera que presiones Enter antes de continuar.
Después de terminar la visualización, sigue con el reporte normal
(lista de tokens, tabla de símbolos, errores) como siempre.

### Pruebas automatizadas

```bash
tests/run_tests.sh
```

Compila el proyecto y corre cada archivo `.lp` de `tests/` contra el
resultado esperado guardado en `tests/expected/`. Imprime `OK` o `FALLO`
por cada prueba y un resumen al final; si algo falla, muestra el `diff`
exacto. Si haces un cambio que **a propósito** cambia la salida (por
ejemplo, al empezar la Semana 3), corre `tests/run_tests.sh --actualizar`
para regenerar `tests/expected/` con el nuevo comportamiento correcto.

Este script usa `bash`, así que en Windows corre desde Git Bash, WSL, o
similar; en Linux/WSL corre directo.

---

## 5. Si la docente pide cambiar algo y volver a compilar

Flujo general, sin salir de pánico:

1. Identifica **qué archivo** hay que tocar (ver la guía
   `EXPLICACION_CODIGO.md` para saber qué hace cada uno).
2. Haz el cambio con cualquier editor (Bloc de notas, VS Code, lo que
   tengas a mano).
3. Guarda el archivo.
4. Vuelve a correr `compilar.bat`. Si compila sin errores, verás
   `Compilado correctamente: LexLP.exe`.
5. Corre de nuevo `LexLP.exe <archivo.lp>` para mostrar el resultado.

Ejemplos típicos de "cambia esto en vivo" y dónde tocar (más detalle en
la otra guía, sección 7):

| Pedido típico | Archivo y lugar |
|---|---|
| Agregar una nueva palabra reservada | `src/Lexer.cpp`, mapa `reservadas` dentro de `esPalabraReservada()` |
| Cambiar el nombre con el que se imprime un token | `src/Token.cpp`, función `nombreTipoToken()` |
| Probar con otro archivo de entrada | no se toca código: `LexLP.exe tests\otro_archivo.lp` |
| Cambiar el archivo por defecto | `src/main.cpp`, línea 98 |

Si el compilador marca error, **lee el mensaje**: g++ indica el archivo y
la línea exacta del problema; casi siempre es una llave, punto y coma o
comilla que falta.
