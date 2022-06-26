# Joc-EDA-FIB
[![HitCount](https://hits.dwyl.com/miquelt9/Joc-EDA.svg?style=flat-square&show=unique)](http://hits.dwyl.com/miquelt9/Joc-EDA)
[![Contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat&show=unique)](/CONTRIBUTING.md)
[![GitHub stars](https://img.shields.io/github/stars/miquelt9/Joc-EDA.svg)](https://GitHub.com/miquelt9/Joc-EDA/stargazers/)
[![GitHub forks](https://img.shields.io/github/forks/miquelt9/Joc-EDA.svg)](https://GitHub.com/miquelt9/Joc-EDA/network/)
[![GitHub repo size in bytes](https://img.shields.io/github/repo-size/miquelt9/Joc-EDA.svg)](https://github.com/miquelt9/Joc-EDA)
[![GitHub contributors](https://img.shields.io/github/contributors/miquelt9/Joc-EDA.svg)](https://GitHub.com/miquelt9/Joc-EDA/graphs/contributors/)

Abans de res, si tu també vas participar t'animem a compartir el codi de la teva IA (pots fer pull request, issue o enviar-me un missatge)

Aquí hi trobareu informació i algunes IA del joc d'[EDA](https://www.cs.upc.edu/eda/) QT 2021-2022 de la [FIB](https://www.fib.upc.edu/)
- Per resumir breument: el joc es diu [Pandemic](https://jutge.org/problems/P41108_en) (creat pel professor Martí Oller) i per guanyar el que has de fer és anar conquerint ciutats i camins situats en diferents punts del mapa. Depenent de la connectivitat d'aquests, s'aconsegueixen més a o menys punts i qui té més punts guanya, però compte també hauràs d'evitar que les teves unitats s'infectin amb un virus letal. Informació completa al document [rules](rules.pdf).
- Per entendre el funcionament de la competició interna de la FIB és necessari llegir [aquest document](joc-cat.pdf).
---
- Al zip [game.zip](game.zip) hi troabreu el joc en si.
- Per crear la teva IA es recomana començar des de AIDemo.cc hi anar escrivint codi per que les unitats duguin a terme les tàctiques requerides.
- Un cop finalitzat la teva IA hauràs de compilar i ja podràs jugar partides.
  - Per compilar abans has de copiar el AIDummy:
  ```
  cp AIDummy.o.Linux64 AIDummy.o
  ```
  - I ara pots fer un make:
  ```
  make
  ```
  - Per veure els jugadors disponibles:
  ```
  ./Game -l
  ```
  - Per jugar (recordar canviar "Nom" pel de la teva IA i "123" per qualsevol numero):
  ```
  ./Game -s 123 Nom Dummy Dummy Dummy < default.cnf > default.out
  ```
  A la terminal apareixerà la informació de la partida i qui ha guanyat.
---
- Per jugar multiples rondes de forma automàtica podeu utilitzar l'script de python [multiple_games.py](multiple_games.py) o [multiple_games3.py](multiple_games3.py), per executar-lo:
```
python3 multiple_games3.py
```
  i seguir els passos que s'indiquen, pot ser que hagis d'instal·lar:
```
pip3 install subprocess
```
---
- A la carptea [Codis IA](Codis%20IA), hi trobareu el codis referents a la propia IA.
- A la carpeta [Jugadors](Jugadors), hi trobareu els arxius compilats d'altres persones per poder jugar amb aquests i comparar-los.
