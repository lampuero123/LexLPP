#include "Parser.h"
#include <cctype>

// ===========================================================================
// Utilidades de recorrido
// ===========================================================================

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0) {
    // Garantiza que la lista termine en FIN_ARCHIVO, asi actual() nunca se
    // sale del vector.
    if (tokens_.empty() || tokens_.back().tipo != TipoToken::FIN_ARCHIVO) {
        int linea = tokens_.empty() ? 1 : tokens_.back().linea;
        int col   = tokens_.empty() ? 1 : tokens_.back().columna + static_cast<int>(tokens_.back().lexema.size());
        tokens_.push_back(Token(TipoToken::FIN_ARCHIVO, "", linea, col));
    }
}

const Token& Parser::actual() const { return tokens_[pos_]; }

const Token& Parser::anterior() const {
    return tokens_[pos_ == 0 ? 0 : pos_ - 1];
}

const Token& Parser::siguiente() const {
    size_t i = pos_ + 1;
    if (i >= tokens_.size()) i = tokens_.size() - 1; // FIN_ARCHIVO
    return tokens_[i];
}

bool Parser::es(TipoToken tipo) const { return actual().tipo == tipo; }

const Token& Parser::consumir() {
    const Token& t = tokens_[pos_];
    if (t.tipo != TipoToken::FIN_ARCHIVO) pos_++;
    return t;
}

// ===========================================================================
// Reporte de errores
// ===========================================================================

// Como se nombra un token ESPERADO en el mensaje.
std::string Parser::descripcionEsperado(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::ID:          return "un identificador";
        case TipoToken::NUM_INT:     return "un numero entero";
        case TipoToken::NUM_DEC:     return "un numero decimal";
        case TipoToken::TEXTO:       return "un texto";
        case TipoToken::COMP:        return "un operador de comparacion (> >= < <= != ==)";
        case TipoToken::FIN_ARCHIVO: return "el fin del archivo";
        default: break;
    }
    // Palabras reservadas: se muestran en minuscula ('main', 'int', ...).
    // Simbolos: el nombre del token ya es el simbolo ( ';' '{' ')' ... ).
    std::string nombre = nombreTipoToken(tipo);
    bool esReservada = !nombre.empty() && std::isalpha(static_cast<unsigned char>(nombre[0]));
    if (esReservada) {
        for (char& c : nombre) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return "'" + nombre + "'";
}

// Como se nombra el token ENCONTRADO en el mensaje.
std::string Parser::descripcionEncontrado(const Token& t) {
    switch (t.tipo) {
        case TipoToken::FIN_ARCHIVO:
            return "el fin del archivo";
        case TipoToken::ID:
        case TipoToken::NUM_INT:
        case TipoToken::NUM_DEC:
        case TipoToken::TEXTO:
        case TipoToken::COMP:
            return "'" + t.lexema + "' (" + nombreTipoToken(t.tipo) + ")";
        default:
            return "'" + t.lexema + "'";
    }
}

// Construye el mensaje y lanza la excepcion. Formato:
//   Error sintactico [linea L, columna C]: se esperaba X pero se encontro Y.
//   <pista opcional>
void Parser::error(const std::string& esperado, const std::string& pista) {
    const Token& t = actual();
    std::string msg = "Error sintactico [linea " + std::to_string(t.linea) +
                      ", columna " + std::to_string(t.columna) + "]: se esperaba " +
                      esperado + " pero se encontro " + descripcionEncontrado(t) + ".";
    if (!pista.empty()) msg += "\n  Pista: " + pista;
    throw ErrorSintactico(msg, t.linea, t.columna);
}

// Consume un token del tipo pedido; si no coincide, reporta el error.
// Cuando falta un delimitador (; ) ] }) y el token encontrado esta en una
// linea posterior, lo mas probable es que se olvido al final de la linea
// anterior: se agrega esa pista porque es el error mas comun.
const Token& Parser::esperar(TipoToken tipo) {
    if (es(tipo)) return consumir();

    std::string pista;
    bool esDelimitador = (tipo == TipoToken::PUNTOYCOMA || tipo == TipoToken::PAR_DER ||
                          tipo == TipoToken::COR_DER    || tipo == TipoToken::LLAVE_DER);
    if (esDelimitador && pos_ > 0 && actual().linea > anterior().linea) {
        pista = "posiblemente falta " + descripcionEsperado(tipo) + " al final de la linea " +
                std::to_string(anterior().linea) + " (despues de '" + anterior().lexema + "').";
    }
    error(descripcionEsperado(tipo), pista);
}

// ===========================================================================
// Conjuntos PRIMERO
// ===========================================================================

bool Parser::esTipoBase() const {
    return es(TipoToken::CHAR) || es(TipoToken::FLOAT) ||
           es(TipoToken::INT)  || es(TipoToken::BOOLEAN);
}

bool Parser::esConstante() const {
    return es(TipoToken::NUM_INT) || es(TipoToken::NUM_DEC);
}

// PRIMERO(COMANDO) = { scanf, println, id, func, if, while, return }
bool Parser::esInicioComando() const {
    switch (actual().tipo) {
        case TipoToken::SCANF:
        case TipoToken::PRINTLN:
        case TipoToken::ID:
        case TipoToken::FUNC:
        case TipoToken::IF:
        case TipoToken::WHILE:
        case TipoToken::RETURN:
            return true;
        default:
            return false;
    }
}

// PRIMERO(FACTOR) = { +, -, id, num_int, num_dec, texto, !, ( }
bool Parser::esInicioFactor() const {
    switch (actual().tipo) {
        case TipoToken::SUMA:
        case TipoToken::RESTA:
        case TipoToken::ID:
        case TipoToken::NUM_INT:
        case TipoToken::NUM_DEC:
        case TipoToken::TEXTO:
        case TipoToken::NOT:
        case TipoToken::PAR_IZQ:
            return true;
        default:
            return false;
    }
}

// ===========================================================================
// Punto de entrada
// ===========================================================================

bool Parser::analizar() {
    pos_ = 0;
    mensajeError_.clear();
    try {
        programa();
    } catch (const ErrorSintactico& e) {
        mensajeError_ = e.what();
        return false;
    }
    return true;
}

// ===========================================================================
// Estructura del programa
// ===========================================================================

// PROGRAMA -> LISTA_FUNCIONES PRINCIPAL
void Parser::programa() {
    listaFunciones();
    principal();
    if (!es(TipoToken::FIN_ARCHIVO)) {
        error("el fin del archivo (main debe ser la ultima funcion del programa)");
    }
}

// LISTA_FUNCIONES -> DECLARACION_FUNCION LISTA_FUNCIONES | epsilon
// PRIMERO(DECLARACION_FUNCION) = { char, float, int, boolean, void }.
// 'void' tambien puede iniciar "void main()": en ese caso se mira un token
// mas adelante (lookahead) y, si es 'main', no es una funcion: es PRINCIPAL.
void Parser::listaFunciones() {
    while (esTipoBase() || (es(TipoToken::VOID) && siguiente().tipo != TipoToken::MAIN)) {
        declaracionFuncion();
    }
}

// DECLARACION_FUNCION -> TIPO_RETORNO id ( PARAMETROS ) BLOQUE
void Parser::declaracionFuncion() {
    tipoRetorno();
    esperar(TipoToken::ID);
    esperar(TipoToken::PAR_IZQ);
    parametros();
    esperar(TipoToken::PAR_DER);
    bloque();
}

// TIPO_RETORNO -> TIPO | void
void Parser::tipoRetorno() {
    if (es(TipoToken::VOID)) consumir();
    else                     tipo();
}

// TIPO -> TIPO_BASE DIMENSION
void Parser::tipo() {
    tipoBase();
    dimension();
}

// TIPO_BASE -> char | float | int | boolean
void Parser::tipoBase() {
    if (esTipoBase()) consumir();
    else error("un tipo ('char', 'float', 'int' o 'boolean')");
}

// DIMENSION -> [ num_int ] DIMENSION | epsilon
void Parser::dimension() {
    while (es(TipoToken::COR_IZQ)) {
        consumir();
        esperar(TipoToken::NUM_INT);
        esperar(TipoToken::COR_DER);
    }
}

// PARAMETROS -> TIPO id LISTA_PARAMETROS | epsilon
void Parser::parametros() {
    if (esTipoBase()) {
        tipo();
        esperar(TipoToken::ID);
        listaParametros();
    }
}

// LISTA_PARAMETROS -> , TIPO id LISTA_PARAMETROS | epsilon
void Parser::listaParametros() {
    while (es(TipoToken::COMA)) {
        consumir();
        tipo();
        esperar(TipoToken::ID);
    }
}

// PRINCIPAL -> main ( ) BLOQUE
//
// EXTENSION: se acepta un 'void' opcional antes de 'main', porque el
// ejemplo oficial del enunciado y las pruebas usan "void main() { ... }"
// aunque la gramatica del PDF escribe solo "main ( )".
void Parser::principal() {
    if (es(TipoToken::VOID)) consumir();
    esperar(TipoToken::MAIN);
    esperar(TipoToken::PAR_IZQ);
    esperar(TipoToken::PAR_DER);
    bloque();
}

// BLOQUE -> { LISTA_VARIABLES COMANDOS }
void Parser::bloque() {
    esperar(TipoToken::LLAVE_IZQ);
    listaVariables();
    comandos();
    if (!es(TipoToken::LLAVE_DER) && esTipoBase()) {
        // Una declaracion aparecio despues de algun comando.
        error("'}'", "las declaraciones de variables deben ir al inicio del bloque, "
                     "antes de cualquier comando.");
    }
    esperar(TipoToken::LLAVE_DER);
}

// ===========================================================================
// Declaraciones
// ===========================================================================

// LISTA_VARIABLES -> TIPO id LISTA_ID ; LISTA_VARIABLES | epsilon
void Parser::listaVariables() {
    while (esTipoBase()) {
        tipo();
        esperar(TipoToken::ID);
        listaId();
        esperar(TipoToken::PUNTOYCOMA);
    }
}

// LISTA_ID -> , id LISTA_ID | epsilon
void Parser::listaId() {
    while (es(TipoToken::COMA)) {
        consumir();
        esperar(TipoToken::ID);
    }
}

// ===========================================================================
// Comandos
// ===========================================================================

// COMANDOS -> COMANDO COMANDOS | epsilon
void Parser::comandos() {
    while (esInicioComando()) comando();
}

// COMANDO -> LECTURA | ESCRITURA | ASIGNACION | FUNCION | SELECCION
//          | MIENTRAS | RETORNO
// Se elige la produccion segun el token actual (gramatica LL(1)).
void Parser::comando() {
    switch (actual().tipo) {
        case TipoToken::SCANF:   lectura();    break;
        case TipoToken::PRINTLN: escritura();  break;
        case TipoToken::ID:      asignacion(); break;
        case TipoToken::FUNC:    funcion();    break;
        case TipoToken::IF:      seleccion();  break;
        case TipoToken::WHILE:   mientras();   break;
        case TipoToken::RETURN:  retorno();    break;
        default:
            error("un comando (scanf, println, asignacion, func, if, while o return)");
    }
}

// LECTURA -> scanf ( TERMINO_LECTURA NUEVO_TERMINO_LECTURA ) ;
void Parser::lectura() {
    esperar(TipoToken::SCANF);
    esperar(TipoToken::PAR_IZQ);
    terminoLectura();
    nuevoTerminoLectura();
    esperar(TipoToken::PAR_DER);
    esperar(TipoToken::PUNTOYCOMA);
}

// TERMINO_LECTURA -> id DIMENSION2
void Parser::terminoLectura() {
    esperar(TipoToken::ID);
    dimension2();
}

// NUEVO_TERMINO_LECTURA -> , TERMINO_LECTURA NUEVO_TERMINO_LECTURA | epsilon
void Parser::nuevoTerminoLectura() {
    while (es(TipoToken::COMA)) {
        consumir();
        terminoLectura();
    }
}

// DIMENSION2 -> [ EXPRESION_ADITIVA ] DIMENSION2 | epsilon
void Parser::dimension2() {
    while (es(TipoToken::COR_IZQ)) {
        consumir();
        expresionAditiva();
        esperar(TipoToken::COR_DER);
    }
}

// ESCRITURA -> println ( TERMINO_ESCRITURA NUEVO_TERMINO_ESCRITURA ) ;
void Parser::escritura() {
    esperar(TipoToken::PRINTLN);
    esperar(TipoToken::PAR_IZQ);
    terminoEscritura();
    nuevoTerminoEscritura();
    esperar(TipoToken::PAR_DER);
    esperar(TipoToken::PUNTOYCOMA);
}

// TERMINO_ESCRITURA -> id DIMENSION2 | CONSTANTE | texto
void Parser::terminoEscritura() {
    if (es(TipoToken::ID)) {
        consumir();
        dimension2();
    } else if (esConstante()) {
        constante();
    } else if (es(TipoToken::TEXTO)) {
        consumir();
    } else {
        error("un identificador, una constante numerica o un texto");
    }
}

// NUEVO_TERMINO_ESCRITURA -> , TERMINO_ESCRITURA NUEVO_TERMINO_ESCRITURA | epsilon
void Parser::nuevoTerminoEscritura() {
    while (es(TipoToken::COMA)) {
        consumir();
        terminoEscritura();
    }
}

// SELECCION -> if ( EXPRESION ) BLOQUE SINO
void Parser::seleccion() {
    esperar(TipoToken::IF);
    esperar(TipoToken::PAR_IZQ);
    expresion();
    esperar(TipoToken::PAR_DER);
    bloque();
    sino();
}

// SINO -> else BLOQUE | epsilon
void Parser::sino() {
    if (es(TipoToken::ELSE)) {
        consumir();
        bloque();
    }
}

// MIENTRAS -> while ( EXPRESION ) BLOQUE
void Parser::mientras() {
    esperar(TipoToken::WHILE);
    esperar(TipoToken::PAR_IZQ);
    expresion();
    esperar(TipoToken::PAR_DER);
    bloque();
}

// ASIGNACION -> id = COMPLEMENTO ;
void Parser::asignacion() {
    esperar(TipoToken::ID);
    esperar(TipoToken::ASIGNACION);
    complemento();
    esperar(TipoToken::PUNTOYCOMA);
}

// COMPLEMENTO -> EXPRESION | FUNCION
void Parser::complemento() {
    if (es(TipoToken::FUNC)) funcion();
    else                     expresion();
}

// FUNCION -> func id ( ARGUMENTOS )
// Nota: segun la gramatica, FUNCION no lleva ';' por si misma. Cuando se
// usa como COMANDO no hay punto y coma; cuando va en una asignacion, el ';'
// lo pone ASIGNACION.
void Parser::funcion() {
    esperar(TipoToken::FUNC);
    esperar(TipoToken::ID);
    esperar(TipoToken::PAR_IZQ);
    argumentos();
    esperar(TipoToken::PAR_DER);
}

// ARGUMENTOS -> EXPRESION NUEVO_ARGUMENTO | epsilon
void Parser::argumentos() {
    if (esInicioFactor()) {
        expresion();
        nuevoArgumento();
    }
}

// NUEVO_ARGUMENTO -> , EXPRESION NUEVO_ARGUMENTO | epsilon
void Parser::nuevoArgumento() {
    while (es(TipoToken::COMA)) {
        consumir();
        expresion();
    }
}

// RETORNO -> return EXPRESION ;
void Parser::retorno() {
    esperar(TipoToken::RETURN);
    expresion();
    esperar(TipoToken::PUNTOYCOMA);
}

// ===========================================================================
// Expresiones (de menor a mayor precedencia: || , && , comp , + - , * / %)
// Cada nivel llama al siguiente; por eso * / % se agrupan antes que + -.
// Los operadores binarios son asociativos a la izquierda (se usan bucles).
// ===========================================================================

// EXPRESION -> EXPRESION_O
void Parser::expresion() { expresionO(); }

// EXPRESION_O -> EXPRESION_Y EXPRESION_O2
void Parser::expresionO() {
    expresionY();
    expresionO2();
}

// EXPRESION_O2 -> || EXPRESION_Y EXPRESION_O2 | epsilon
void Parser::expresionO2() {
    while (es(TipoToken::OR)) {
        consumir();
        expresionY();
    }
}

// EXPRESION_Y -> EXPRESION_RELACIONAL EXPRESION_Y2
void Parser::expresionY() {
    expresionRelacional();
    expresionY2();
}

// EXPRESION_Y2 -> && EXPRESION_RELACIONAL EXPRESION_Y2 | epsilon
void Parser::expresionY2() {
    while (es(TipoToken::AND)) {
        consumir();
        expresionRelacional();
    }
}

// EXPRESION_RELACIONAL -> EXPRESION_ADITIVA EXPRESION_RELACIONAL2
void Parser::expresionRelacional() {
    expresionAditiva();
    expresionRelacional2();
}

// EXPRESION_RELACIONAL2 -> comp EXPRESION_ADITIVA | epsilon
// (la gramatica admite UNA sola comparacion: "a < b < c" no es valido)
void Parser::expresionRelacional2() {
    if (es(TipoToken::COMP)) {
        consumir();
        expresionAditiva();
    }
}

// EXPRESION_ADITIVA -> EXPRESION_MULTIPLICATIVA EXPRESION_ADITIVA2
void Parser::expresionAditiva() {
    expresionMultiplicativa();
    expresionAditiva2();
}

// EXPRESION_ADITIVA2 -> OPERADOR_ADITIVO EXPRESION_MULTIPLICATIVA EXPRESION_ADITIVA2 | epsilon
void Parser::expresionAditiva2() {
    while (es(TipoToken::SUMA) || es(TipoToken::RESTA)) {
        operadorAditivo();
        expresionMultiplicativa();
    }
}

// OPERADOR_ADITIVO -> + | -
void Parser::operadorAditivo() {
    if (es(TipoToken::SUMA) || es(TipoToken::RESTA)) consumir();
    else error("'+' o '-'");
}

// EXPRESION_MULTIPLICATIVA -> FACTOR EXPRESION_MULTIPLICATIVA2
void Parser::expresionMultiplicativa() {
    factor();
    expresionMultiplicativa2();
}

// EXPRESION_MULTIPLICATIVA2 -> OPERADOR_MULTIPLICATIVO FACTOR EXPRESION_MULTIPLICATIVA2 | epsilon
void Parser::expresionMultiplicativa2() {
    while (es(TipoToken::MULT) || es(TipoToken::DIV) || es(TipoToken::MOD)) {
        operadorMultiplicativo();
        factor();
    }
}

// OPERADOR_MULTIPLICATIVO -> * | / | %
void Parser::operadorMultiplicativo() {
    if (es(TipoToken::MULT) || es(TipoToken::DIV) || es(TipoToken::MOD)) consumir();
    else error("'*', '/' o '%'");
}

// FACTOR -> SIGNO TERMINO | texto | ! FACTOR | ( EXPRESION )
void Parser::factor() {
    if (es(TipoToken::SUMA) || es(TipoToken::RESTA) ||
        es(TipoToken::ID)   || esConstante()) {
        signo();
        termino();
    } else if (es(TipoToken::TEXTO)) {
        consumir();
    } else if (es(TipoToken::NOT)) {
        consumir();
        factor();
    } else if (es(TipoToken::PAR_IZQ)) {
        consumir();
        expresion();
        esperar(TipoToken::PAR_DER);
    } else {
        error("una expresion (identificador, numero, texto, '!' o '(')");
    }
}

// TERMINO -> id DIMENSION2 | CONSTANTE
void Parser::termino() {
    if (es(TipoToken::ID)) {
        consumir();
        dimension2();
    } else if (esConstante()) {
        constante();
    } else {
        error("un identificador o una constante numerica");
    }
}

// SIGNO -> + | - | epsilon
void Parser::signo() {
    if (es(TipoToken::SUMA) || es(TipoToken::RESTA)) consumir();
}

// CONSTANTE -> num_int | num_dec
void Parser::constante() {
    if (esConstante()) consumir();
    else error("un numero entero o decimal");
}
