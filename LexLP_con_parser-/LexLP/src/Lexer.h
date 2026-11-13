#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "Token.h"

// ---------------------------------------------------------------------------
// Analizador lexico de LP.
//
// Funciona como un automata implementado a mano: recorre el texto fuente
// caracter por caracter y, segun el caracter actual, decide que token esta
// empezando. No se usa ninguna libreria de expresiones regulares: las ER de
// la especificacion estan traducidas directamente a codigo.
//
// FASE ACTUAL (Semana 3): numeros, identificadores, texto, palabras
// reservadas, operadores, simbolos especiales y comentarios de linea.
// Cualquier caracter que no encaje en las ER de esta fase se reporta
// como ERROR_LEXICO.
// ---------------------------------------------------------------------------
class Lexer {
public:
    explicit Lexer(const std::string& fuente);

    // Reconoce y devuelve el siguiente token del programa fuente.
    // Al llegar al final devuelve un token de tipo FIN_ARCHIVO.
    Token siguienteToken();

    // Recorre todo el fuente y devuelve la lista de tokens validos,
    // en el mismo orden en que aparecen. Los errores quedan aparte
    // y se consultan con errores().
    std::vector<Token> analizarTodo();

    // Variante para el analizador sintactico: igual que analizarTodo() pero
    // el ultimo token de la lista es FIN_ARCHIVO.
    std::vector<Token> analizarConFin();

    const std::vector<Token>& errores() const { return errores_; }

    // Identificadores unicos encontrados, en orden de aparicion.
    // El indice de cada uno es el 'atributo' que llevan sus tokens ID.
    const std::vector<std::string>& tablaSimbolos() const { return tablaSimbolos_; }

private:
    std::string fuente_;   // todo el contenido del archivo .lp
    size_t      pos_;      // posicion del caracter que se esta mirando
    int         linea_;    // linea actual  (empieza en 1)
    int         columna_;  // columna actual(empieza en 1)

    std::vector<Token>       errores_;
    std::vector<std::string> tablaSimbolos_;

    // --- utilidades de recorrido ---
    bool finEntrada() const;
    char actual() const;        // caracter actual sin consumirlo
    char siguiente() const;     // caracter de adelante (lookahead)
    char avanzar();             // consume el caracter actual y actualiza linea/columna

    void omitirEspacios();      // espacios, tabulaciones y saltos de linea
    void omitirComentario();    // "//" hasta el fin de la linea (o del archivo)

    // --- reconocedores (uno por cada ER de la especificacion) ---
    Token leerNumero();             // implementa  D+  y  D+\.D+
    Token leerIdentificador();      // implementa  L(L|D)*  y detecta palabras reservadas
    Token leerTexto();              // implementa  ".*"
    Token leerOperadorOSimbolo();   // operadores (=,+,-,*,/,%,&&,||,!,>,>=,<,<=,!=,==)
                                     // y simbolos especiales ( ) [ ] { } , ;

    // Busca 'lexema' en la tabla de simbolos; si no esta, lo agrega.
    // Devuelve el indice en ambos casos.
    int indiceSimbolo(const std::string& lexema);

    // Si 'lexema' es una palabra reservada de LP, deja su tipo en 'tipo'
    // y devuelve true. Si no, devuelve false (es un ID comun).
    static bool esPalabraReservada(const std::string& lexema, TipoToken& tipo);

    static bool esDigito(char c);   // D = [0-9]
    static bool esLetra(char c);    // L = [a-zA-Z_]
};

#endif // LEXER_H
