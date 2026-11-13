#ifndef PARSER_H
#define PARSER_H

#include <stdexcept>
#include <string>
#include <vector>
#include "Token.h"

// ---------------------------------------------------------------------------
// Error sintactico: se lanza como excepcion desde cualquier funcion del
// parser y se atrapa en Parser::analizar(). Lleva la ubicacion del token
// donde se detecto el problema.
// ---------------------------------------------------------------------------
struct ErrorSintactico : public std::runtime_error {
    int linea;
    int columna;
    ErrorSintactico(const std::string& mensaje, int linea, int columna)
        : std::runtime_error(mensaje), linea(linea), columna(columna) {}
};

// ---------------------------------------------------------------------------
// Analizador sintactico de LP por DESCENSO RECURSIVO.
//
// Recibe la lista de tokens que produce el Lexer (nunca vuelve a mirar los
// caracteres del fuente). Hay una funcion por cada no terminal de la
// gramatica LL(1) del PDF "Gramatica_LP_LL1", con el mismo nombre en
// minusculas/camelCase:
//
//      PROGRAMA -> programa()        EXPRESION_ADITIVA -> expresionAditiva()
//      BLOQUE   -> bloque()          LISTA_ID          -> listaId()   ...
//
// Las producciones con epsilon se implementan con un 'if' / 'while' que
// pregunta si el token actual pertenece a PRIMERO de la produccion; si no,
// se toma epsilon (no se consume nada).
//
// Convencion de los terminales de la gramatica -> TipoToken del lexer:
//      id -> ID     num_int -> NUM_INT     num_dec -> NUM_DEC
//      texto -> TEXTO     comp -> COMP     func -> FUNC
// ---------------------------------------------------------------------------
class Parser {
public:
    // 'tokens' es la salida de Lexer::analizarConFin() (termina en FIN_ARCHIVO).
    explicit Parser(const std::vector<Token>& tokens);

    // Analiza el programa completo. Devuelve true si es sintacticamente
    // correcto (ACEPTADO) y false si se detecto un error (RECHAZADO).
    bool analizar();

    // Mensaje del error detectado (vacio si el programa fue aceptado).
    const std::string& mensajeError() const { return mensajeError_; }

private:
    std::vector<Token> tokens_;
    size_t             pos_;          // indice del token actual
    std::string        mensajeError_;

    // --- utilidades de recorrido ---
    const Token& actual() const;
    const Token& anterior() const;    // ultimo token consumido
    const Token& siguiente() const;   // lookahead de un token mas
    bool         es(TipoToken tipo) const;
    const Token& consumir();          // avanza y devuelve el token consumido
    const Token& esperar(TipoToken tipo);   // consume 'tipo' o reporta error

    // --- reporte de errores ---
    [[noreturn]] void error(const std::string& esperado, const std::string& pista = "");
    static std::string descripcionEsperado(TipoToken tipo);
    static std::string descripcionEncontrado(const Token& t);

    // --- conjuntos PRIMERO usados para decidir producciones ---
    bool esTipoBase() const;       // char | float | int | boolean
    bool esConstante() const;      // num_int | num_dec
    bool esInicioComando() const;
    bool esInicioFactor() const;

    // --- un metodo por cada no terminal de la gramatica ---
    void programa();
    void listaFunciones();
    void declaracionFuncion();
    void tipoRetorno();
    void tipo();
    void tipoBase();
    void dimension();
    void parametros();
    void listaParametros();
    void principal();
    void bloque();
    void listaVariables();
    void listaId();
    void comandos();
    void comando();
    void lectura();
    void terminoLectura();
    void nuevoTerminoLectura();
    void dimension2();
    void escritura();
    void terminoEscritura();
    void nuevoTerminoEscritura();
    void seleccion();
    void sino();
    void mientras();
    void asignacion();
    void complemento();
    void funcion();
    void argumentos();
    void nuevoArgumento();
    void retorno();
    void expresion();
    void expresionO();
    void expresionO2();
    void expresionY();
    void expresionY2();
    void expresionRelacional();
    void expresionRelacional2();
    void expresionAditiva();
    void expresionAditiva2();
    void operadorAditivo();
    void expresionMultiplicativa();
    void expresionMultiplicativa2();
    void operadorMultiplicativo();
    void factor();
    void termino();
    void signo();
    void constante();
};

#endif // PARSER_H
