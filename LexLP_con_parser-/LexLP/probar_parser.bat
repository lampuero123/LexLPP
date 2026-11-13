@echo off
REM Corre todas las pruebas del analizador sintactico en Windows.
REM Requisito: haber ejecutado antes compilar.bat (genera ParserLP.exe).
REM   - tests\sintactico\validos\*.lp    deben ser ACEPTADOS  (codigo 0)
REM   - tests\sintactico\invalidos\*.lp  deben ser RECHAZADOS (codigo 1)

if not exist ParserLP.exe (
    echo No existe ParserLP.exe. Ejecuta primero compilar.bat
    exit /b 2
)

set TOTAL=0
set FALLAS=0

echo --- Programas que deben ser ACEPTADOS ---
for %%f in (tests\sintactico\validos\*.lp) do (
    ParserLP.exe "%%f" >nul
    if errorlevel 1 (
        echo FALLO   %%f
        set /a FALLAS+=1 >nul
    ) else (
        echo OK      %%f
    )
    set /a TOTAL+=1 >nul
)

echo.
echo --- Programas que deben ser RECHAZADOS ---
for %%f in (tests\sintactico\invalidos\*.lp) do (
    ParserLP.exe "%%f" >nul
    if errorlevel 2 (
        echo FALLO   %%f
        set /a FALLAS+=1 >nul
    ) else if errorlevel 1 (
        echo OK      %%f
    ) else (
        echo FALLO   %%f
        set /a FALLAS+=1 >nul
    )
    set /a TOTAL+=1 >nul
)

echo.
echo Total: %TOTAL% pruebas, %FALLAS% fallidas.
if %FALLAS% neq 0 exit /b 1
