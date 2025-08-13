# Gramaticas-Presentacion-1-David-Castellanos

# CFG Recognizer — Python & C

>  **Reconocedor de gramáticas libres de contexto (CFG)**.  
> Dado un archivo con una gramática (formato BNF simple) y un archivo con cadenas de prueba (una por línea), el programa imprime por cada cadena **`acepta`** si la gramática la genera o **`NO acepta`** si no la genera.

---

##  Badges (opcional)
`![Python](https://img.shields.io/badge/lang-Python%20%26%20C-blue)` `![License](https://img.shields.io/badge/license-MIT-green)`

---

# Contenido del repositorio
- `cfg_recognizer.py` — **Python** (recursivo + memoización).  
- `cfg_recognizer.c` — **C** (requiere compilador con C99 o superior).  
- `cfg_recognizer_fixed.c` — **C** (versión compatible con compiladores antiguos).  
- `grammar_G1.txt` … `grammar_G5.txt` — gramáticas de ejemplo (cada archivo: primera línea = símbolo inicial).  
- `tests_G1.txt` … `tests_G5.txt` — archivos de prueba (una cadena por línea; línea vacía = ε).  
- `README.md` — este archivo.  
- `README.txt` — instrucciones cortas (opcional).

---

#  Formato de la gramática (`grammar_*.txt`)

- **Primera línea**: símbolo inicial (por ejemplo `S`).  
- Cada producción en una línea con el formato:

  ```
  A -> a B | b | ε
  ```

  - `|` separa alternativas.  
  - `ε` representa la cadena vacía (también acepta `e` o alternativa vacía).  
  - <u>**No terminales**</u>: identificadores que comienzan con mayúscula (`S`, `A`, `Expr`).  
  - <u>**Terminales**</u>: caracteres que no empiezan en mayúscula; el recognizer trata terminales como **un solo carácter** (p. ej. `a`, `b`, `0`).  
  - Líneas que empiezan con `#` se ignoran (comentarios).

**Ejemplo** (`grammar_G1.txt`):
```
S
S -> a S b | ε
```

---

#  Formato de tests (`tests_G*.txt`)

- Una cadena por línea.  
- Una **línea vacía** representa la cadena vacía `""` (ε).  
- Evita comentarios dentro de este archivo.

**Ejemplo** (`tests_G1.txt`):
```
ab
aabb
aaabbb
aab
aba

```
(la última línea vacía corresponde a ε)

---

#  Idea del algoritmo (explicación clara y compacta)

Se define la función recursiva:

**`reachable(NT, pos)` → conjunto de posiciones** que se pueden alcanzar en la cadena si derivamos desde el no-terminal `NT` empezando en la posición `pos`.

- Para una producción `X Y a Z`:
  - Si el símbolo es **terminal**, se exige coincidencia con el carácter actual y se avanza.
  - Si es **no-terminal**, se llama recursivamente para obtener todas las posiciones alcanzables desde ahí y se sigue con los símbolos restantes.
  - Si la producción es `ε`, se agrega `pos` como posición alcanzable.
- Se hace **unión** por todas las producciones de `NT`.
- **Memoización**: guardamos `reachable(NT,pos)` ya calculado para evitar recomputar (imprescindible para gramáticas recursivas).
- La cadena `s` se acepta si `len(s)` pertenece a `reachable(S, 0)`.

---

#  Ejecución — Python

Requiere Python 3.

```bash
python3 cfg_recognizer.py grammar_G1.txt tests_G1.txt
```

Salida: una línea por cada entrada en `tests_G1.txt` con `acepta` o `NO acepta`.

---

# 🛠️ Compilar y ejecutar — C

Si tu compilador soporta C99/C11 (recomendado):

```bash
gcc -std=c99 cfg_recognizer.c -o cfg_recognizer
./cfg_recognizer grammar_G1.txt tests_G1.txt
```

Si aparece el error `for loop initial declarations are only allowed in C99 or C11 mode`, usa la versión fija:

```bash
gcc cfg_recognizer_fixed.c -o cfg_recognizer
./cfg_recognizer grammar_G1.txt tests_G1.txt
```

En Windows (PowerShell / CMD):

```powershell
cfg_recognizer.exe grammar_G1.txt tests_G1.txt
```

---

# 🔍 Ejemplo de salida (G1)

**Gramática** (`grammar_G1.txt`):
```
S
S -> a S b | ε
```

**Tests** (`tests_G1.txt`):
```
ab
aabb
aaabbb
aab
aba

```

**Salida esperada**:
```
acepta
acepta
acepta
NO acepta
NO acepta
acepta
```
(la última `acepta` corresponde a la línea vacía → ε)

---

# ⚠️ Notas y limitaciones importantes

- <u>Terminales</u> se tratan como **un solo carácter**. Para tokens multi-caracter (ej. `id`, `num`) es necesario un preprocesamiento (lexer) o ajustar el parser del formato de gramática.  
- El recognizer responde solo **sí/no**; no construye el árbol de derivación.  
- Para entradas y gramáticas muy grandes la memoria puede crecer (cada `reachable(NT,pos)` guarda un array de tamaño `n+1`). Para producción se recomiendan algoritmos como **CYK** (si se transforma a CNF), **Earley** o GLR.  
- Se asume que los archivos de gramática están bien formados (validación mínima).

---
