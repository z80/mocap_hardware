@echo off
setlocal
cd %~dp0
call .\.venv\Scripts\activate
:loop
python main.py
if %ERRORLEVEL% neq 0 (
    echo Script terminated unexpectedly. Restarting...
)
goto loop
endlocal
