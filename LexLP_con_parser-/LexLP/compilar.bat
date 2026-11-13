@echo off
REM Compila LexLP (analizador lexico) y ParserLP (analizador sintactico)
REM con MinGW-w64 (g++).
REM -static: los .exe quedan autocontenidos y corren sin DLLs adicionales.

set GXX=g++
where g++ >nul 2>nul || set GXX=C:\mingw64\bin\g++.exe

"%GXX%" -std=c++17 -Wall -Wextra -static -o LexLP.exe src\main.cpp src\Lexer.cpp src\Token.cpp
if %errorlevel% neq 0 (
    echo.
    echo Fallo la compilacion de LexLP.
    exit /b %errorlevel%
)

"%GXX%" -std=c++17 -Wall -Wextra -static -o ParserLP.exe src\mainParser.cpp src\Parser.cpp src\Lexer.cpp src\Token.cpp
if %errorlevel% neq 0 (
    echo.
    echo Fallo la compilacion de ParserLP.
    exit /b %errorlevel%
)
echo Compilado correctamente: LexLP.exe y ParserLP.exe
