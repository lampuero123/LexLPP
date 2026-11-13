#include "Token.h"

std::string nombreTipoToken(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::NUM_INT:      return "NUM_INT";
        case TipoToken::NUM_DEC:      return "NUM_DEC";
        case TipoToken::ID:           return "ID";
        case TipoToken::TEXTO:        return "TEXTO";

        case TipoToken::INT:          return "INT";
        case TipoToken::FLOAT:        return "FLOAT";
        case TipoToken::CHAR:         return "CHAR";
        case TipoToken::BOOLEAN:      return "BOOLEAN";
        case TipoToken::VOID:         return "VOID";
        case TipoToken::IF:           return "IF";
        case TipoToken::ELSE:         return "ELSE";
        case TipoToken::FOR:          return "FOR";
        case TipoToken::WHILE:        return "WHILE";
        case TipoToken::SCANF:        return "SCANF";
        case TipoToken::PRINTLN:      return "PRINTLN";
        case TipoToken::MAIN:         return "MAIN";
        case TipoToken::RETURN:       return "RETURN";
        case TipoToken::FUNC:         return "FUNC";

        case TipoToken::ASIGNACION:   return "=";
        case TipoToken::SUMA:         return "+";
        case TipoToken::RESTA:        return "-";
        case TipoToken::MULT:         return "*";
        case TipoToken::DIV:          return "/";
        case TipoToken::MOD:          return "%";
        case TipoToken::AND:          return "&&";
        case TipoToken::OR:           return "||";
        case TipoToken::NOT:          return "!";
        // Las 6 comparaciones (> >= < <= != ==) comparten una sola categoria
        // generica COMP: asi lo pide la especificacion (seccion 4.3 y el
        // ejemplo de la seccion 12, donde ">=" y "<=" imprimen ambas <COMP>).
        case TipoToken::COMP:         return "COMP";

        case TipoToken::PAR_IZQ:      return "(";
        case TipoToken::PAR_DER:      return ")";
        case TipoToken::COR_IZQ:      return "[";
        case TipoToken::COR_DER:      return "]";
        case TipoToken::LLAVE_IZQ:    return "{";
        case TipoToken::LLAVE_DER:    return "}";
        case TipoToken::COMA:         return ",";
        case TipoToken::PUNTOYCOMA:   return ";";

        case TipoToken::ERROR_LEXICO: return "ERROR_LEXICO";
        case TipoToken::FIN_ARCHIVO:  return "FIN_ARCHIVO";
    }
    return "DESCONOCIDO";
}

Token::Token(TipoToken tipo,
             const std::string& lexema,
             int linea,
             int columna,
             int atributo)
    : tipo(tipo), lexema(lexema), atributo(atributo), linea(linea), columna(columna) {}

std::string Token::aCadena() const {
    // Solo ID lleva atributo (posicion en la tabla de simbolos): se imprime
    // como <ID,0>. El resto de tokens se imprime solo con su nombre.
    if (atributo >= 0) {
        return "<" + nombreTipoToken(tipo) + "," + std::to_string(atributo) + ">";
    }
    return "<" + nombreTipoToken(tipo) + ">";
}
