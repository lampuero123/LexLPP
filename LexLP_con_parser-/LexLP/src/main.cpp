// ---------------------------------------------------------------------------
// LexLP - Analizador Lexico para el lenguaje LP
// Curso: Compiladores
//
// AVANCE SEMANA 1 (15/09): lectura del archivo fuente + reconocimiento de
//                          NUM_INT y NUM_DEC + reporte de errores lexicos
//                          con linea y columna.
// AVANCE SEMANA 2 (22/09): ID + TEXTO + palabras reservadas + tabla de
//                          simbolos.
// AVANCE SEMANA 3 (29/09): operadores + simbolos especiales + comentarios
//                          + integracion completa del token set de LP.
//
// Funcionalidades adicionales (no forman parte del automata, solo de la
// presentacion en consola):
//   - Coloreado sintactico basico de la lista/detalle de tokens y errores.
//   - Modo de visualizacion paso a paso del reconocimiento (--visualizar).
//
// Uso:  LexLP.exe  <archivo.lp>  [--visualizar]
//       Si no se indica archivo, usa tests/prueba_semana1.lp
// ---------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// windows.h define las macros VOID y ERROR, que chocan con los tokens
// TipoToken::VOID y color::ERROR. Se desactivan despues del include
// (solo se usan GetStdHandle/SetConsoleMode, que no las necesitan).
#undef VOID
#undef ERROR
#endif

#include "Lexer.h"
#include "Token.h"

// ---------------------------------------------------------------------------
// Coloreado sintactico basico (codigos ANSI). Solo se usa al imprimir en
// consola (std::cout); los archivos exportados en output/ quedan siempre en
// texto plano, sin codigos de color, para que se puedan abrir en cualquier
// editor sin ruido.
// ---------------------------------------------------------------------------
namespace color {
    const std::string RESET      = "\033[0m";
    const std::string NUMERO     = "\033[36m";   // cian:    NUM_INT / NUM_DEC
    const std::string ID         = "\033[33m";   // amarillo: identificadores
    const std::string TEXTO      = "\033[32m";   // verde:   cadenas TEXTO
    const std::string RESERVADA  = "\033[35m";   // magenta: palabras reservadas
    const std::string OPERADOR   = "\033[34m";   // azul:    operadores
    const std::string SIMBOLO    = "\033[37m";   // blanco:  simbolos especiales ( ) { } [ ] , ;
    const std::string ERROR      = "\033[1;31m"; // rojo brillante: errores lexicos
}

// En Windows, cmd.exe necesita habilitar explicitamente el procesamiento de
// secuencias ANSI; en Linux/WSL la terminal ya las soporta de forma nativa.
static void habilitarColoresConsola() {
#ifdef _WIN32
    HANDLE hSalida = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (hSalida != INVALID_HANDLE_VALUE && GetConsoleMode(hSalida, &modo)) {
        SetConsoleMode(hSalida, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

// Color asociado a cada tipo de token, segun su categoria.
static const std::string& colorParaTipo(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::NUM_INT:
        case TipoToken::NUM_DEC:
            return color::NUMERO;
        case TipoToken::ID:
            return color::ID;
        case TipoToken::TEXTO:
            return color::TEXTO;
        case TipoToken::ERROR_LEXICO:
            return color::ERROR;
        case TipoToken::INT:
        case TipoToken::FLOAT:
        case TipoToken::CHAR:
        case TipoToken::BOOLEAN:
        case TipoToken::VOID:
        case TipoToken::IF:
        case TipoToken::ELSE:
        case TipoToken::FOR:
        case TipoToken::WHILE:
        case TipoToken::SCANF:
        case TipoToken::PRINTLN:
        case TipoToken::MAIN:
        case TipoToken::RETURN:
        case TipoToken::FUNC:
            return color::RESERVADA;
        case TipoToken::ASIGNACION:
        case TipoToken::SUMA:
        case TipoToken::RESTA:
        case TipoToken::MULT:
        case TipoToken::DIV:
        case TipoToken::MOD:
        case TipoToken::AND:
        case TipoToken::OR:
        case TipoToken::NOT:
        case TipoToken::COMP:
            return color::OPERADOR;
        case TipoToken::PAR_IZQ:
        case TipoToken::PAR_DER:
        case TipoToken::COR_IZQ:
        case TipoToken::COR_DER:
        case TipoToken::LLAVE_IZQ:
        case TipoToken::LLAVE_DER:
        case TipoToken::COMA:
        case TipoToken::PUNTOYCOMA:
            return color::SIMBOLO;
        default:
            return color::RESET; // FIN_ARCHIVO: no deberia llegar a imprimirse
    }
}

// Lee TODO el archivo fuente y lo devuelve en un solo string.
// Se lee completo (y no linea por linea) porque el analizador necesita poder
// mirar el caracter siguiente aunque este al final de una linea.
static bool leerArchivo(const std::string& ruta, std::string& contenido) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        return false;
    }
    std::stringstream buffer;
    buffer << archivo.rdbuf();
    contenido = buffer.str();
    return true;
}

// Imprime la lista de tokens respetando el orden del programa fuente
// y agrupandolos por linea, tal como los muestra la especificacion.
// 'colorear' solo debe ser true cuando 'salida' es la consola.
static void imprimirListaTokens(std::ostream& salida, const std::vector<Token>& tokens,
                                 bool colorear = false) {
    int lineaActual = -1;
    for (const Token& t : tokens) {
        if (t.linea != lineaActual) {
            if (lineaActual != -1) salida << "\n";
            lineaActual = t.linea;
        } else {
            salida << " ";
        }
        if (colorear) salida << colorParaTipo(t.tipo) << t.aCadena() << color::RESET;
        else          salida << t.aCadena();
    }
    salida << "\n";
}

// Detalle de cada token con su ubicacion: sirve de apoyo para depurar
// y para sustentar el reconocimiento en la presentacion.
static void imprimirDetalleTokens(std::ostream& salida, const std::vector<Token>& tokens,
                                   bool colorear = false) {
    salida << "LINEA  COLUMNA  TOKEN         LEXEMA\n";
    salida << "-----  -------  ------------  ------------\n";
    for (const Token& t : tokens) {
        salida << "  " << t.linea
               << "\t   " << t.columna
               << "\t   ";
        if (colorear) salida << colorParaTipo(t.tipo) << nombreTipoToken(t.tipo) << color::RESET;
        else          salida << nombreTipoToken(t.tipo);
        salida << "\t " << t.lexema << "\n";
    }
}

// Tabla de simbolos: un identificador por linea, en el orden en que se
// registraron (ese orden es el mismo indice que aparece como atributo
// en los tokens <ID,n>).
static void imprimirTablaSimbolos(std::ostream& salida, const std::vector<std::string>& tabla) {
    if (tabla.empty()) {
        salida << "No se encontraron identificadores.\n";
        return;
    }
    salida << "INDICE  IDENTIFICADOR\n";
    salida << "------  -------------\n";
    for (size_t i = 0; i < tabla.size(); ++i) {
        salida << "  " << i << "\t   " << tabla[i] << "\n";
    }
}

static void imprimirErrores(std::ostream& salida, const std::vector<Token>& errores,
                             bool colorear = false) {
    if (errores.empty()) {
        salida << "No se encontraron errores lexicos.\n";
        return;
    }
    salida << "LINEA  COLUMNA  LEXEMA  RESULTADO\n";
    salida << "-----  -------  ------  -------------\n";
    for (const Token& e : errores) {
        salida << "  " << e.linea
               << "\t   " << e.columna
               << "\t   ";
        if (colorear) salida << color::ERROR << e.lexema << color::RESET;
        else          salida << e.lexema;
        salida << "\t " << nombreTipoToken(e.tipo) << "\n";
    }
}

// Busca en 'fuente' la posicion (indice de caracter) donde empieza la
// linea/columna dada. Un token solo trae (linea, columna), no su posicion
// dentro del string completo; esta funcion la reconstruye recorriendo el
// texto igual que lo hace el propio Lexer con avanzar().
static size_t posicionDesdeLineaColumna(const std::string& fuente, int lineaObjetivo, int columnaObjetivo) {
    int linea = 1, columna = 1;
    for (size_t i = 0; i < fuente.size(); ++i) {
        if (linea == lineaObjetivo && columna == columnaObjetivo) return i;
        if (fuente[i] == '\n') { linea++; columna = 1; }
        else                   { columna++; }
    }
    return fuente.size();
}

// ---------------------------------------------------------------------------
// Modo de visualizacion (--visualizar): recorre el archivo token por token,
// igual que analizarTodo(), pero deteniendose en cada paso para mostrar que
// parte del texto se acaba de reconocer. Usa su propio Lexer (independiente
// del que hace el analisis "real"), asi que no interfiere con el resto del
// programa ni con los archivos exportados en output/.
// ---------------------------------------------------------------------------
static void modoVisualizacion(const std::string& fuente) {
    std::cout << "\n=== VISUALIZACION PASO A PASO ===\n";
    std::cout << "(presiona Enter despues de cada paso)\n";

    Lexer lexer(fuente);
    int paso = 0;

    while (true) {
        Token t = lexer.siguienteToken();
        if (t.tipo == TipoToken::FIN_ARCHIVO) {
            std::cout << "\n--- Fin del archivo: no quedan mas caracteres por leer ---\n";
            break;
        }

        paso++;
        size_t inicio = posicionDesdeLineaColumna(fuente, t.linea, t.columna);
        size_t fin    = inicio + t.lexema.size();

        std::cout << "\nPaso " << paso << ":\n  ";
        std::cout << fuente.substr(0, inicio);                                   // ya procesado
        std::cout << colorParaTipo(t.tipo) << fuente.substr(inicio, fin - inicio) // lexema actual
                   << color::RESET;
        std::cout << fuente.substr(fin);                                         // todavia sin leer
        std::cout << "\n";

        if (t.tipo == TipoToken::ERROR_LEXICO) {
            std::cout << "  -> " << color::ERROR << "ERROR_LEXICO" << color::RESET;
        } else {
            std::cout << "  -> " << colorParaTipo(t.tipo) << t.aCadena() << color::RESET;
        }
        std::cout << "   lexema=\"" << t.lexema << "\""
                   << "   (linea " << t.linea << ", columna " << t.columna << ")\n";

        std::cout << "  [Enter para continuar] " << std::flush;
        std::cin.get();
    }
}

int main(int argc, char* argv[]) {
    std::string rutaFuente = "tests/prueba_semana1.lp";
    bool modoVisual = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--visualizar" || arg == "-v") {
            modoVisual = true;
        } else {
            rutaFuente = arg;
        }
    }

    habilitarColoresConsola();

    std::string fuente;
    if (!leerArchivo(rutaFuente, fuente)) {
        std::cerr << "ERROR: no se pudo abrir el archivo '" << rutaFuente << "'\n";
        return 1;
    }

    std::cout << "=== LexLP - Analizador Lexico ===\n";
    std::cout << "Archivo fuente: " << rutaFuente << "\n\n";

    if (modoVisual) {
        modoVisualizacion(fuente);
    }

    // 1. Analisis lexico
    Lexer lexer(fuente);
    std::vector<Token> tokens = lexer.analizarTodo();

    // 2. Salida por consola (coloreada)
    std::cout << "--- LISTA DE TOKENS ---\n";
    imprimirListaTokens(std::cout, tokens, true);

    std::cout << "\n--- DETALLE (linea / columna) ---\n";
    imprimirDetalleTokens(std::cout, tokens, true);

    std::cout << "\n--- TABLA DE SIMBOLOS ---\n";
    imprimirTablaSimbolos(std::cout, lexer.tablaSimbolos());

    std::cout << "\n--- ERRORES LEXICOS ---\n";
    imprimirErrores(std::cout, lexer.errores(), true);

    std::cout << "\nTotal de tokens reconocidos: " << tokens.size() << "\n";
    std::cout << "Total de identificadores:    " << lexer.tablaSimbolos().size() << "\n";
    std::cout << "Total de errores lexicos:    " << lexer.errores().size() << "\n";

    // 3. Exportacion de resultados a la carpeta output/
    std::filesystem::create_directories("output");

    std::ofstream fTokens("output/tokens.txt");
    imprimirListaTokens(fTokens, tokens);
    fTokens << "\n";
    imprimirDetalleTokens(fTokens, tokens);
    fTokens << "\n--- TABLA DE SIMBOLOS ---\n";
    imprimirTablaSimbolos(fTokens, lexer.tablaSimbolos());

    std::ofstream fErrores("output/errores.txt");
    imprimirErrores(fErrores, lexer.errores());

    std::cout << "\nResultados exportados a output/tokens.txt y output/errores.txt\n";
    return 0;
}
