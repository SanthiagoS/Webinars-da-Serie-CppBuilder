TSX / Delphi System - Integração Portaria v1

ERP Delphi API:
  GET  http://127.0.0.1:3101/api
  GET  http://127.0.0.1:3101/api/health
  GET  http://127.0.0.1:3101/api/employees
  POST http://127.0.0.1:3101/api/access/events

Primeiro teste:
1. Compile e execute prjDelphiSystem.
2. Abra no navegador:
   http://127.0.0.1:3101/api
3. Depois:
   http://127.0.0.1:3101/api/employees
4. No C++Builder, Configurações > ERP Delphi:
   http://127.0.0.1:3101/api
   e use TESTAR CONEXÃO.

Observação:
A API Delphi usa porta 3101.
A função APIBaseURL existente no Delphi continua apontando para a API do terminal
C++ na porta 3000; portanto as duas direções podem coexistir.
