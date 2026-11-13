#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Pruebas del analizador sintactico (ParserLP).
#   tests/sintactico/validos/*.lp    -> deben ser ACEPTADOS  (exit 0)
#   tests/sintactico/invalidos/*.lp  -> deben ser RECHAZADOS (exit 1)
# Uso:  tests/run_tests_parser.sh        (agregar -v para ver cada mensaje)
# ---------------------------------------------------------------------------
cd "$(dirname "$0")/.."
BIN="$(mktemp)"
g++ -std=c++17 -Wall -Wextra -o "$BIN" src/mainParser.cpp src/Parser.cpp src/Lexer.cpp src/Token.cpp || exit 2

total=0; fallidas=0
verificar() {   # $1 = archivo, $2 = exit esperado
    total=$((total + 1))
    salida=$("$BIN" "$1"); codigo=$?
    if [[ $codigo -eq $2 ]]; then echo "OK      $1"; else echo "FALLO   $1 (exit $codigo, esperado $2)"; fallidas=$((fallidas + 1)); fi
    if [[ "${2:-}" == "1" && "${VERBOSE:-}" == "1" ]]; then echo "$salida" | grep -A1 "^Error\|^  linea" | sed 's/^/        /'; fi
}
[[ "${1:-}" == "-v" ]] && VERBOSE=1
for f in tests/sintactico/validos/*.lp;   do verificar "$f" 0; done
for f in tests/sintactico/invalidos/*.lp; do verificar "$f" 1; done
rm -f "$BIN"
echo; echo "Total: $total pruebas, $fallidas fallidas."
[[ $fallidas -eq 0 ]]
