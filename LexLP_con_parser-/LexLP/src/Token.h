#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// ---------------------------------------------------------------------------
// Tipos de token del lenguaje LP.
//
// Variables base de la especificacion:
//      D = [0-9]
//      L = [a-zA-Z_]
//
// FASE ACTUAL (Semana 3): numericos + identificadores + texto + palabras
// reservadas + tabla de simbolos + operadores + simbolos especiales +
// comentarios.
// ---------------------------------------------------------------------------
enum class TipoToken {
    NUM_INT,        // ER:  D+                 ejemplo: 20
    NUM_DEC,        // ER:  D+ \. D+            ejemplo: 15.5
    ID,             // ER:  L (L|D)*            ejemplo: edad
    TEXTO,          // ER:  " .* "              ejemplo: "hola"

    // Palabras reservadas
    INT, FLOAT, CHAR, BOOLEAN, VOID,
    IF, ELSE, FOR, WHILE,
    SCANF, PRINTLN, MAIN, RETURN, FUNC,

    // Operadores
    ASIGNACION,     // =
    SUMA, RESTA, MULT, DIV, MOD,   // + - * / %
    AND, OR, NOT,                  // && || !
    COMP,           // > >= < <= != ==  (categoria generica, spec 4.3)

    // Simbolos especiales
    PAR_IZQ, PAR_DER,      // ( )
    COR_IZQ, COR_DER,      // [ ]
    LLAVE_IZQ, LLAVE_DER,  // { }
    COMA, PUNTOYCOMA,      // , ;

    ERROR_LEXICO,   // caracter (o cadena) que no pertenece al alfabeto de LP
    FIN_ARCHIVO     // marca interna: se llego al final de la entrada
};

// Nombre del tipo de token tal como debe aparecer en los reportes.
std::string nombreTipoToken(TipoToken tipo);

// ---------------------------------------------------------------------------
// Un token reconocido en el programa fuente.
// 'atributo' guarda la posicion en la tabla de simbolos cuando el token es
// un identificador (ID). Para el resto de tokens vale -1 = no aplica.
// ---------------------------------------------------------------------------
struct Token {
    TipoToken   tipo;
    std::string lexema;    // el texto exacto que se leyo del archivo
    int         atributo;  // posicion en la tabla de simbolos, -1 si no aplica
    int         linea;     // linea donde inicia el lexema (empieza en 1)
    int         columna;   // columna donde inicia el lexema (empieza en 1)

    Token(TipoToken tipo,
          const std::string& lexema,
          int linea,
          int columna,
          int atributo = -1);

    // Formato pedido para la lista de tokens:  <NUM_INT>   y luego  <ID,0>
    std::string aCadena() const;
};

#endif // TOKEN_H
