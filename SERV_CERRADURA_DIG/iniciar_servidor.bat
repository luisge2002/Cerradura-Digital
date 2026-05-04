@echo off
cd /d "D:\PROYECTOS ELECTRONICOS\CERRADURA DIGITAL\SERV_CERRADURA_DIG"

start "Servidor Cerradura" /min cmd /c ".venv\Scripts\python.exe servidor_cerradura.py >> servidor.log 2>&1"
exit