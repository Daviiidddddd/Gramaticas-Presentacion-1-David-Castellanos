#!/usr/bin/env python3
# cfg_recognizer.py
# Usage: python3 cfg_recognizer.py grammar_Gi.txt tests_Gi.txt
# Prints "acepta" or "NO acepta" for each test line.

import sys
from collections import defaultdict

EPS = "ε"

def parse_grammar(path):
    with open(path, encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f if ln.strip() != "" and not ln.strip().startswith("#")]
    if not lines:
        raise ValueError("Gramática vacía")
    start = lines[0].strip()
    prods = defaultdict(list)
    for ln in lines[1:]:
        if "->" not in ln:
            continue
        left, right = ln.split("->", 1)
        left = left.strip()
        for alt in right.split("|"):
            sym_seq = alt.strip()
            if sym_seq == "" or sym_seq == EPS:
                prods[left].append([])
            else:
                tokens = []
                i = 0
                while i < len(sym_seq):
                    c = sym_seq[i]
                    if c.isspace():
                        i += 1
                        continue
                    if c.isupper():
                        j = i+1
                        while j < len(sym_seq) and (sym_seq[j].isalnum() or sym_seq[j]=='_'):
                            j += 1
                        tokens.append(sym_seq[i:j])
                        i = j
                    else:
                        tokens.append(c)
                        i += 1
                prods[left].append(tokens)
    return start, prods

def recognize(s, start, prods):
    n = len(s)
    memo = {}

    def reachable(nt, pos):
        key = (nt, pos)
        if key in memo:
            return memo[key]
        res = set()
        for prod in prods.get(nt, []):
            current_positions = {pos}
            for sym in prod:
                next_positions = set()
                for cp in current_positions:
                    if cp > n:
                        continue
                    if sym.isupper():
                        child_pos = reachable(sym, cp)
                        next_positions.update(child_pos)
                    else:
                        if cp < n and s[cp] == sym:
                            next_positions.add(cp+1)
                current_positions = next_positions
                if not current_positions:
                    break
            else:
                if not prod:
                    res.add(pos)
                else:
                    res.update(current_positions)
        memo[key] = res
        return res

    final_positions = reachable(start, 0)
    return n in final_positions

def run(grammar_path, tests_path):
    start, prods = parse_grammar(grammar_path)
    with open(tests_path, encoding="utf-8") as f:
        for raw in f:
            line = raw.rstrip("\n")
            inp = line
            ok = recognize(inp, start, prods)
            print("acepta" if ok else "NO acepta")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Uso: python3 cfg_recognizer.py grammar_Gi.txt tests_Gi.txt")
        sys.exit(1)
    run(sys.argv[1], sys.argv[2])
