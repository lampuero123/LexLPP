#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Pruebas automatizadas de LexLP.
#
# Compila el analizador y corre cada archivo tests/*.lp que tenga un
# resultado esperado guardado en tests/expected/. Compara la salida
# generada (output/tokens.txt y output/errores.txt) contra ese resultado
# con 'diff'. Si algun archivo de prueba nuevo se agrega sin su version
# esperada en tests/expected/, se omite (no cuenta como fallo).
#
# Uso:
#   tests/run_tests.sh              corre todas las pruebas
#   tests/run_tests.sh --actualizar regenera tests/expected/ con la salida
#                                    actual (usar solo cuando un cambio de
#                                    comportamiento es intencional)
# ---------------------------------------------------------------------------
set -euo pipefail
cd "$(dirname "$0")/.."

BIN="$(mktemp)"
g++ -std=c++17 -Wall -Wextra -o "$BIN" src/main.cpp src/Lexer.cpp src/Token.cpp

ACTUALIZAR=false
if [[ "${1:-}" == "--actualizar" ]]; then
    ACTUALIZAR=true
fi

total=0
fallidas=0

for archivo in tests/*.lp; do
    base=$(basename "$archivo" .lp)
    esperado_tokens="tests/expected/$base.tokens.txt"
    esperado_errores="tests/expected/$base.errores.txt"

    "$BIN" "$archivo" > /dev/null

    if $ACTUALIZAR; then
        cp output/tokens.txt "$esperado_tokens"
        cp output/errores.txt "$esperado_errores"
        echo "ACTUALIZADO  $base"
        continue
    fi

    if [[ ! -f "$esperado_tokens" || ! -f "$esperado_errores" ]]; then
        echo "OMITIDO      $base (sin resultado esperado en tests/expected/)"
        continue
    fi

    total=$((total + 1))
    ok=true
    if ! diff -u "$esperado_tokens" output/tokens.txt > /tmp/lexlp_diff_tokens.txt 2>&1; then
        ok=false
    fi
    if ! diff -u "$esperado_errores" output/errores.txt > /tmp/lexlp_diff_errores.txt 2>&1; then
        ok=false
    fi

    if $ok; then
        echo "OK           $base"
    else
        echo "FALLO        $base"
        if [[ -s /tmp/lexlp_diff_tokens.txt ]]; then
            echo "  --- diferencia en tokens.txt ---"
            sed 's/^/  /' /tmp/lexlp_diff_tokens.txt
        fi
        if [[ -s /tmp/lexlp_diff_errores.txt ]]; then
            echo "  --- diferencia en errores.txt ---"
            sed 's/^/  /' /tmp/lexlp_diff_errores.txt
        fi
        fallidas=$((fallidas + 1))
    fi
done

rm -f "$BIN" /tmp/lexlp_diff_tokens.txt /tmp/lexlp_diff_errores.txt

if $ACTUALIZAR; then
    exit 0
fi

echo ""
echo "Total: $total pruebas, $fallidas fallidas."
[[ $fallidas -eq 0 ]]
