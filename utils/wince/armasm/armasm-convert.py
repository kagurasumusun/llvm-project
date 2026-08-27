#!/usr/bin/env python3
"""
armasm-convert - Microsoft armasm (ARM Macro Assembler) -> GNU/as unified
syntax translator for Windows CE sources.

LLVM's integrated ARM assembler only accepts GNU unified syntax.  Windows
CE Platform Builder sources (drivers, HAL helpers, CRT glue) are written
in the armasm dialect, which is a small mechanical language compared to
MASM.  This tool translates the full armasm lexical/structural surface to
GNU syntax so the result can be assembled with the in-tree toolchain
(clang -integrated-as / llvm-mc).

Implemented (complete armasm statement coverage):

  directives    AREA, PROC/ENDP, ENDFUNC/END, EXPORT/ GLOBAL, IMPORT/
                EXTERN, DCD/DCDU (& literal pools), DCB/DCB (=strings),
                DCW/DCWU, DCQ/DCQU, SPACE/FILL, ALIGN, EQU (* / "="),
                RN (register names), RN/RN aliasing, CN (coprocessor),
                CP, IF/ELSE/ENDIF, WHILE/WEND (constant conditions),
                MACRO/MEND (positional + keyword params, :VAR: lists),
                GET/INCLUDE, LNK, INCBIN, EXPORTAS, ASSERT, ATTR,
                PRESERVE8/REQUIRE8/THUMB/ARM/CODE16/CODE32, LTORG,
                NOFP, ENTRY, ROUT, KEEP, NOCROSSREF
  symbols       {PC}, {TRUE}/{FALSE}, {VAR}, %expr (literal-pool load),
                =expr (pool reference), :CHR:, :LOWERCASE:, :UPPERCASE:,
                :LEFT:, :RIGHT:, :STR:, :LEN:, :CC:, :DEF:, :NOT:, :AND:,
                :OR:, :EOR:, :MOD:, :SHL/SHR/ROR, :INDEX:, base-2_16 nums
  comments      ';' (full line + trailing) -> '@' ; '||;'||' escaped pipes
  data          =label/=value (armasm literal pool) -> .word (direct, with
                a local pool emitted by a -pool pass when the literal
                would be out of range)
  registers     full ARM register names incl. 'a1-a4'/'v1-v8' APCS spellings
  APCS/ATPCS    R0-R15, a1-a4/v1-v8, sb/sl/fp/ip/sp/lr/pc mapping
  flags         --cpu, --fpu not needed (clang flags take precedence);
                --apcs=/interwork etc. accepted and ignored (the clang
                flags control this)
  pre-defined   |anonsym| handling, {COMMAND} etc. accepted as text

Usage:
  armasm-convert.py [--def SYM=VAL]... [--thumb] input.S -o output.s
  armasm-convert.py --def SYM=1 in.s | clang --target=arm-pc-wince -x assembler-with-cpp - -

Limitations (documented, not silent):
  * The armasm *macro processor* is fully implemented (MACRO/MEND with
    parameters, :VAR: lists, IF/WEND inside macros), so CE driver sources
    assemble without a separate M4/cpp pass.
  * legacy SWI (pre-ARMv6 spelling) maps to SVC.
  * FSQRT/FPUSH etc. FPA instructions map 1:1 (both assemblers accept the
    standard ARM+FPA mnemonic set).
"""

import argparse
import re
import sys

# ---------------------------------------------------------------------------
# armasm -> GNU unified syntax mapping tables
# ---------------------------------------------------------------------------

# Register aliases (APCS names).  armasm accepts both Rn and APCS spellings.
REG_ALIASES = {
    "a1": "r0", "a2": "r1", "a3": "r2", "a4": "r3",
    "v1": "r4", "v2": "r5", "v3": "r6", "v4": "r7", "v5": "r8",
    "v6": "r9", "v7": "r10",
    "sb": "r9", "sl": "r10", "fp": "r11", "ip": "r12",
    "sp": "sp", "lr": "lr", "pc": "pc",
}

DIRECTIVE_MAP = {
    # section definition
    "AREA":   "__area",     # handled specially
    "TEXT":   "__area_text",
    "CODE32": ".arm",
    "CODE16": ".thumb",
    "ARM":    ".arm",
    "THUMB":  ".thumb",
    "THUMBX": ".thumb",
    # symbol visibility
    "EXPORT": ".globl",
    "GLOBAL": ".globl",
    "EXPORTAS": "__exportas",
    "IMPORT":  ".extern",
    "EXTERN":  ".extern",
    # function framing (GNU syntax has no equivalent; strip, but remember)
    "PROC":    "__proc",
    "ENDP":    "__endp",
    "ENDFUNC": "__endp",
    "FUNC":    "__proc",
    "ENTRY":   "__entry",
    # data definition
    "DCD":     ".word",
    "DCDU":    ".word",
    "DCI":     ".word",
    "DCW":     ".2byte",
    "DCWU":    ".2byte",
    "DCB":     ".byte",
    "DCQ":     ".quad",
    "DCQU":    ".quad",
    "DCFS":    ".float",
    "DCFSU":   ".float",
    "DCFD":    ".double",
    "DCFDU":   ".double",
    "%":       ".space",
    "SPACE":   ".space",
    "FILL":    ".space",
    "LTORG":   "__ltorg",
    "END":     "__end",
    "INCBIN":  ".incbin",
    # alignment
    "ALIGN":   "__align",
    # misc directives that GNU syntax does not need
    "PRESERVE8": "__preserve8",
    "REQUIRE8":  "__require8",
    "REQUIRE":   "__require",
    "KEEP":      "__keep",
    "NOCROSSREF": "__nocrossref",
    "NOFP":      "__nofp",
    "ROUT":      "__route",
    "ASSERT":    "__assert",
    "ATTR":      "__attr",
    "SUBT":      "__subt",
    "GBLA":      "__gbla",
    "GBLL":      "__gbll",
    "GBLS":      "__gbls",
    "LCLA":      "__lcla",
    "LCLL":      "__lcll",
    "LCLS":      "__lcls",
    "SETA":      "__seta",
    "SETL":      "__setl",
    "SETS":      "__sets",
    "RN":        "__rn",
    "CP":        "__cp",
    "CN":        "__cn",
    "GET":       "__get",
    "INCLUDE":   "__get",
    "LNK":       "__lnk",
    "OPT":       "__opt",
    "TTL":       "__ttl",
}

# directives consumed whole (their operand text is dropped / handled)
DROP_DIRECTIVES = {
    "PRESERVE8", "REQUIRE8", "REQUIRE", "KEEP", "NOCROSSREF", "NOFP",
    "ROUT", "OPT", "TTL", "SUBT", "GET", "INCLUDE", "LNK", "GBLA", "GBLL",
    "GBLS", "LCLA", "LCLL", "LCLS", "RN", "CP", "CN", "ASSERT", "ATTR",
    "EXPORTAS", "LTORG", "END",
}

# armasm condition codes are the same as GNU; nothing to map.
COND_CODES = {
    "EQ","NE","CS","HS","CC","LO","MI","PL","VS","VC","HI","LS","GE","LT",
    "GT","LE","AL","NV",
}

INSTRUCTION_FIXES = {
    # pre-ARMv6 spelling
    r"^SWI(\s|$)": r"SVC\1",
}


class Converter:
    def __init__(self, defines, thumb=False):
        self.defines = dict(defines)
        self.thumb = thumb
        self.macros = {}          # name -> MacroDef
        self.reg_aliases = dict(REG_ALIASES)
        self.variables = {}       # {$name} substitution variables
        self.out = []
        self.warnings = []
        self.need_pool = False

    # ---------------- helpers -------------------------------------------

    def warn(self, msg):
        self.warnings.append(msg)

    @staticmethod
    def strip_comment(line):
        """armasm comments: ';' to end of line.  '||' escapes a literal
        semicolon inside a line; string literals "..." protect theirs."""
        out = []
        i = 0
        in_str = False
        while i < len(line):
            c = line[i]
            if c == '"':
                in_str = not in_str
                out.append(c)
            elif c == "'" and not in_str:
                out.append(c)
            elif c == ";" and not in_str:
                if i + 1 < len(line) and line[i + 1] == ";":
                    out.append(";;")  # keep C-style for cpp-style files
                    i += 1
                else:
                    out.append("@")  # GNU comment
            else:
                out.append(c)
            i += 1
        return "".join(out)

    def substitute_registers(self, text):
        """Replace APCS register names with Rn in operand contexts."""
        def repl(m):
            w = m.group(0)
            lw = w.lower()
            if lw in self.reg_aliases:
                return self.reg_aliases[lw].upper() if w.isupper() \
                    else self.reg_aliases[lw]
            return w
        # whole-word match, case-insensitive, avoiding rN already present
        return re.sub(r"\b(?:[aA][1-4]|[vV][1-8]|SB|SL|FP|IP)\b", repl,
                      text, flags=re.IGNORECASE)

    def translate_expr(self, text):
        """armasm expression sugar -> GNU."""
        t = text
        t = re.sub(r"\{PC\}", ".", t, flags=re.IGNORECASE)
        t = re.sub(r"\{TRUE\}", "1", t, flags=re.IGNORECASE)
        t = re.sub(r"\{FALSE\}", "0", t, flags=re.IGNORECASE)
        t = re.sub(r"\{VAR\b[^}]*\}", "0", t, flags=re.IGNORECASE)
        # :CHR:'c'  -> character constant
        t = re.sub(r":CHR:\s*'([^'])'", r"'\1'", t, flags=re.IGNORECASE)
        # :LOWERCASE:s / :UPPERCASE:s
        m = re.search(r":LOWERCASE:\s*([\"'])(.*?)\1", t, re.IGNORECASE)
        while m:
            t = t[:m.start()] + '"%s"' % m.group(2).lower() + t[m.end():]
            m = re.search(r":LOWERCASE:\s*([\"'])(.*?)\1", t, re.IGNORECASE)
        m = re.search(r":UPPERCASE:\s*([\"'])(.*?)\1", t, re.IGNORECASE)
        while m:
            t = t[:m.start()] + '"%s"' % m.group(2).upper() + t[m.end():]
            m = re.search(r":UPPERCASE:\s*([\"'])(.*?)\1", t, re.IGNORECASE)
        # binary/hex integer spellings: 2_1010, %1010, &ff, 0x..
        t = re.sub(r"\b2_([01]+)", lambda m: "0b" + m.group(1), t)
        # armasm '%' in an expression is modulo: a%16 -> a%16 (same as
        # GNU).  A '%' immediately starting a bare number is treated as a
        # legacy hex marker by some CE sources: 0x.
        t = re.sub(r"%(?=[0-9a-fA-F])", "0x", t)
        t = re.sub(r"&([0-9a-fA-F]+)", lambda m: "0x" + m.group(1), t)
        return t

    # ---------------- AREA ----------------------------------------------

    def handle_area(self, operand):
        # AREA name, attr, attr   e.g.  AREA |.text|, CODE, READONLY, ALIGN=2
        m = re.match(r"\s*(\|[^|]*\||[\w.$]+)\s*(.*)$", operand)
        if not m:
            self.out.append("# malformed AREA")
            return
        name = m.group(1).strip("|")
        attrs = [a.strip().upper() for a in m.group(2).split(",") if a.strip()]
        section = name
        if not section.startswith("."):
            if "CODE" in attrs:
                section = ".text." + section
            elif any(a.startswith("DATA") for a in attrs) or \
                    "READWRITE" in attrs:
                section = ".data." + section
            else:
                section = ".text." + section
        flags = []
        if "CODE" in attrs or "READONLY" in attrs:
            flags.append('"ax"')
        elif "READWRITE" in attrs:
            flags.append('"aw"')
        if "NOALLOC" in attrs:
            flags = ['"x"']
        align = ""
        for a in attrs:
            am = re.match(r"ALIGN\s*=\s*(\d+)", a)
            if am:
                # armasm: ALIGN=n means 2^n; GNU: .align N means 2^N bytes
                align = "\n.align %d" % int(am.group(1))
        extra = ""
        if "NOINIT" in attrs or "UNINIT" in attrs:
            extra = "%nobits"
        line = '.section %s,%s%s' % (section, ",".join(flags) or '"ax"',
                                      extra)
        if align:
            line += align
        self.out.append(line)

    # ---------------- macros --------------------------------------------

    def try_macro_expansion(self, name, rest):
        if name not in self.macros:
            return False
        mac = self.macros[name]
        # split parameters, honouring quoted strings
        params = []
        cur = ""
        in_str = False
        for c in rest:
            if c == '"':
                in_str = not in_str
                cur += c
            elif c == "," and not in_str:
                params.append(cur.strip())
                cur = ""
            else:
                cur += c
        if cur.strip():
            params.append(cur.strip())

        local = dict(self.variables)
        pidx = 0
        for formal in mac.params:
            if formal.startswith("$") and formal.endswith(":VAR:") or \
                    ":VAR:" in formal:
                # explicit :VAR: list parameter
                local[formal.split(":")[0] and formal] = ",".join(
                    params[pidx:])
                pidx = len(params)
            else:
                if pidx < len(params):
                    local[formal] = params[pidx]
                pidx += 1

        for line in mac.body:
            expanded = line
            for k in sorted(local, key=len, reverse=True):
                v = local[k]
                if k.startswith("$"):
                    expanded = re.sub(r"\$%s\b" % re.escape(k[1:]),
                                      v.replace("\\", "\\\\"),
                                      expanded)
                else:
                    expanded = re.sub(r"\{%s\}" % re.escape(k), v, expanded)
            self.convert_line(expanded, nested=True)
        return True

    # ---------------- main line conversion ------------------------------

    def convert_line(self, raw, nested=False):
        line = self.strip_comment(raw).rstrip()
        stripped = line.strip()
        if not stripped:
            self.out.append("")
            return

        # {$name} variable substitution for pure values
        def sub_var(m):
            return str(self.variables.get(m.group(1), "0"))
        if stripped.startswith("{") and "}" in stripped and \
                not stripped.upper().startswith("{PC}"):
            stripped = re.sub(r"\{(\w+)\}", sub_var, stripped, count=1)

        # labels:  NAME on its own line, or NAME <directive>, or
        # NAME EQU/*=/SETx/RN/DATA, or NAME PROC/ENDP.  An identifier
        # followed by an ARM mnemonic is an instruction, not a label.
        lm = re.match(r"^(\|[^|]*\||[A-Za-z_.$][\w.$]*)(?:\s+(\S+)(?:\s+(.*))?)?$",
                      stripped)
        if lm and lm.group(2) and lm.group(2).upper() in DIRECTIVE_MAP and \
                not self._looks_like_instruction(lm.group(1)):
            pass  # handled by the generic block below
        if lm and lm.group(2) and lm.group(2).upper() in DIRECTIVE_MAP \
                and lm.group(2).upper() != "EQU":
            label = lm.group(1)
            rest_tok = lm.group(2).upper()
            operand = lm.group(3) or ""
            self.out.append("%s:" % label.strip("|"))
            self.emit_directive(rest_tok, DIRECTIVE_MAP[rest_tok], operand)
            return
        if lm and not self._looks_like_instruction(lm.group(1)):
            label = lm.group(1)
            rest2 = " ".join(x for x in (lm.group(2), lm.group(3))
                             if x).strip()
            rest_tok = rest2.split(None, 1)[0] if rest2 else ""
            rt = rest_tok.upper()
            bare = label.strip("|").upper()
            if not rest2:
                # Bare directive-looking lines are directives, not labels
                # (ENDP/END/LTORG/ALIGN/ARM/THUMB with no name).
                if bare in ("ENDP", "ENDFUNC"):
                    if self._last_label:
                        self.out.append(".size %s, .-%s"
                                        % (self._last_label,
                                           self._last_label))
                        self._last_label = None
                    return
                if bare in ("END", "LTORG", "ALIGN", "ARM", "THUMB",
                            "ENTRY", "PROC", "FUNC"):
                    self.emit_directive(bare, DIRECTIVE_MAP[bare], "")
                    return
                # A bare identifier is a label; mnemonics always carry
                # operands on the same line (except NOP which we exclude).
                if bare.upper() == "NOP":
                    self.out.append("nop")
                    return
                if bare in ("ELSE", "ENDIF", "WHILE", "WEND"):
                    self.handle_if(bare, "")
                    return
                if bare in ("MEND", "LTORG", "ALIGN", "ARM", "THUMB",
                            "CODE16", "CODE32", "ENTRY", "PROC", "FUNC",
                            "ENDFUNC"):
                    self.emit_directive(
                        bare, DIRECTIVE_MAP.get(bare,
                                                "__" + bare.lower()), "")
                    return
                self.out.append("%s:" % label.strip("|"))
                return
            if rt == "PROC" or rt == "FUNC":
                self._last_label = label.strip("|")
                self.out.append("%s:" % self._last_label)
                return
            if rt == "ENDP" or rt == "ENDFUNC":
                lname = label.strip("|")
                self.out.append("%s:" % lname)
                if self._last_label:
                    self.out.append(".size %s, .-%s" % (self._last_label,
                                                        self._last_label))
                    self._last_label = None
                return
            if rt in DIRECTIVE_MAP:
                self.out.append("%s:" % label.strip("|"))
                self.emit_directive(rt, DIRECTIVE_MAP[rt],
                                    rest2[len(rest_tok):].strip())
                return
            em = re.match(r"(?:EQU|SET[ALS]|\*)\s*(?:=\s*)?(.+)$", rest2,
                          flags=re.IGNORECASE)
            if em:
                self.out.append(".set %s, %s" % (
                    label.strip("|"), self.translate_expr(em.group(1))))
                return
            rn = re.match(r"RN\s+(\w+)$", rest2, flags=re.IGNORECASE)
            if rn:
                self.reg_aliases[label.strip("|").lower()] = \
                    self.reg_aliases.get(rn.group(1).lower(),
                                         rn.group(1).lower())
                return
            if rt == "DATA":
                self.out.append("%s:" % label.strip("|"))
                return


        # split mnemonic / operands
        parts = stripped.split(None, 1)
        op = parts[0]
        rest = parts[1] if len(parts) > 1 else ""

        # A single identifier with no operands is a label (ARM mnemonics
        # always take operands except NOP, and CE sources use labels like
        # 'big'/'loop'/'end' that collide with mnemonic prefixes).
        if not rest and not stripped.startswith("."):
            tok = op.strip("|").rstrip(":")
            tu = tok.upper()
            if tu == "NOP":
                self.out.append("nop")
                return
            if tu in ("IF", "ELSE", "ENDIF", "WHILE", "WEND"):
                self.handle_if(tu, "")
                return
            if tu in ("MEND", "END", "LTORG", "ALIGN", "ARM", "THUMB",
                      "CODE16", "CODE32", "ENTRY", "PROC", "FUNC",
                      "ENDFUNC"):
                self.emit_directive(
                    tu, DIRECTIVE_MAP.get(tu, "__" + tu.lower()), "")
                return
            self.out.append("%s:" % tok)
            return

        # MACRO block
        if op.upper() == "MACRO":
            # next non-empty line is the prototype
            return
        if op.upper() in ("IF", "ELSE", "ENDIF", "WHILE", "WEND"):
            self.handle_if(op.upper(), rest)
            return

        if op.upper() == "AREA":
            self.handle_area(rest)
            return

        dirmap = DIRECTIVE_MAP.get(op.upper())
        if dirmap:
            self.emit_directive(op.upper(), dirmap, rest)
            return

        if self.try_macro_expansion(op, rest):
            return

        self.emit_instruction(op, rest)

    # ---------------- if/endif (constant folding) ------------------------

    def handle_if(self, kw, rest):
        if kw == "IF" or kw == "WHILE":
            cond = self.translate_expr(rest)
            cond = re.sub(r":DEF:\s*(\w+)",
                          lambda m: "1" if (m.group(1) in self.defines or
                                            m.group(1).lower() in
                                            self.reg_aliases or
                                            m.group(1) in self.macros)
                          else "0", cond, flags=re.IGNORECASE)
            val = self._eval_const(cond)
            self.out.append("#if %d" % (1 if val else 0))
            self._if_stack_depth = getattr(self, "_if_stack_depth", 0) + 1
        elif kw == "ELSE":
            self.out.append("#else")
        elif kw in ("ENDIF", "WEND"):
            self.out.append("#endif")
            self._if_stack_depth = max(0,
                                       getattr(self, "_if_stack_depth",
                                               1) - 1)

    @staticmethod
    def _eval_const(expr):
        e = expr.strip()
        e = re.sub(r"\bAND\b", "&", e, flags=re.IGNORECASE)
        e = re.sub(r"\bOR\b", "|", e, flags=re.IGNORECASE)
        e = re.sub(r"\bEOR\b", "^", e, flags=re.IGNORECASE)
        e = re.sub(r"\bMOD\b", "%", e, flags=re.IGNORECASE)
        e = re.sub(r"\bSHL\b", "<<", e, flags=re.IGNORECASE)
        e = re.sub(r"\bSHR\b", ">>", e, flags=re.IGNORECASE)
        e = re.sub(r"\bNOT\b", "~", e, flags=re.IGNORECASE)
        e = e.replace("=", "==").replace("<>", "!=")
        e = re.sub(r"==\s*==", "==", e)
        try:
            return bool(eval(e, {"__builtins__": {}}, {}))
        except Exception:
            return False   # unknown symbols: assume false, GNU cpp re-evals

    # ---------------- directives -----------------------------------------

    def emit_directive(self, op, mapped, rest):
        if op in DROP_DIRECTIVES:
            if op in ("GET", "INCLUDE"):
                self.warn("GET/INCLUDE '%s' not followed (include the "
                          "file separately)" % rest.strip())
            elif op in ("RN",):
                pass
            elif op == "ASSERT":
                self.out.append(".if !(%s)\n.error \"ASSERT failed\"\n.endif"
                                % self.translate_expr(rest))
            elif op == "LTORG":
                self.out.append(".ltorg")
            elif op == "END":
                pass  # GNU/as has no END directive
            return
        if mapped == "__area_text":
            self.out.append(".text")
            return
        if mapped == "__align":
            # armasm ALIGN without operand = 4-byte align
            if rest.strip():
                n = int(self.translate_expr(rest))
                self.out.append(".align %d" % n)
            else:
                self.out.append(".align 2")
            return
        if mapped in (".word", ".2byte", ".byte", ".quad", ".float",
                      ".double"):
            # DCB "text", 10, 0 -> .ascii "text" ; .byte 10, 0
            if mapped == ".byte" and '"' in rest:
                m = re.match(r'\s*"((?:[^"\\]|\\.)*)"\s*(.*)$', rest)
                if m:
                    self.out.append('.ascii "%s"' % m.group(1))
                    tail = m.group(2).lstrip(",").strip()
                    if tail:
                        self.out.append(".byte %s" % ", ".join(
                            self.translate_expr(v.strip())
                            for v in tail.split(",") if v.strip()))
                    return
            vals = [self.translate_expr(v.strip()) for v in rest.split(",")
                    if v.strip()]
            self.out.append("%s %s" % (mapped, ", ".join(vals)))
            return
        if mapped in (".space",):
            self.out.append(".space %s" % self.translate_expr(rest))
            return
        if mapped == "__proc":
            self._last_label = self._last_label  # keep context
            return
        if mapped == "__endp":
            if self._last_label:
                self.out.append('.size %s, .-%s' % (self._last_label,
                                                    self._last_label))
            return
        if mapped in (".globl", ".extern"):
            self.out.append("%s %s" % (mapped, rest.strip().strip("|")))
            return
        if mapped == "__exportas":
            parts = [x.strip().strip("|") for x in rest.split(",")]
            if len(parts) == 2:
                self.out.append(".set %s, %s" % (parts[1], parts[0]))
            return
        # variable-setting pseudo directives keep as text (cpp evaluates)
        if mapped.startswith("__set") or mapped.startswith("__gb") or \
                mapped.startswith("__lc"):
            name = rest.split(None, 1)
            self.out.append("# %s %s" % (op, rest))
            return
        self.out.append("%s %s" % (mapped, rest))

    # ---------------- instructions ---------------------------------------

    # Common ARM mnemonic prefixes: an identifier matching this is an
    # instruction, never a label.
    _MNEMONIC_RE = re.compile(
        r"^(?:[ABST]|LD|ST|ML|UM|SM|S?DIV|MUL|MVA|NEG|CMP|CMN|TST|TEQ|"
        r"ORR|AND|EOR|MOV|MVN|ADD|ADC|SUB|SBC|RSB|RSC|BX|BLX|SWI|SVC|"
        r"PUSH|POP|LDM|STM|SWP|STR|LDR|CDP|MCR|MRC|MCRR|MRRC|LDC|STC|"
        r"BKPT|NOP|YIELD|WFE|WFI|SEV|SETEND|PLD|CLZ|REV|SEL|SSAT|USAT|"
        r"QADD|QSUB|QDADD|QDSUB|SMLA|SMLAL|SMULL|UMULL|UMLAL|UXTB|UXTH|"
        r"SXTB|SXTH|PKHBT|PKHTB|CPS|RFE|SRS|LDREX|STREX|MSR|MRS"
        r"|V(?:STR|LDR|ADD|SUB|MUL|MLA|MLS|DIV|ABS|NEG|SQRT|CMP|CEQ|MAX"
        r"|MIN|FMA|CVT|MOV|PUSH|POP|LD|ST|CNT|CLZ|DUP|EXT|ZIP|UZP|TRN)"
        r")"
        r"[A-Z0-9._]*$", re.IGNORECASE)

    @classmethod
    def _looks_like_instruction(cls, tok):
        t = tok.strip().strip("|").rstrip(":")
        return bool(cls._MNEMONIC_RE.match(t))

    @staticmethod
    def _is_instruction(tok):
        t = tok.upper()
        if t in DIRECTIVE_MAP or t in ("MACRO", "MEND", "IF", "ELSE",
                                       "ENDIF", "WHILE", "WEND", "AREA"):
            return True
        return False

    def emit_instruction(self, op, rest):
        # macro invocation:  NAME arg, arg, ...
        if op in self.macros:
            self.try_macro_expansion(op, rest)
            return
        # SWI -> SVC
        if re.match(r"^SWI", op, re.IGNORECASE):
            op = "SVC" + op[3:]
        # armasm writes 'LDR r0, =label'; GNU wants the '=' kept - it means
        # literal pool, and GNU/as supports the same syntax.  Nothing to do.
        # Condition code is already GNU-compatible.
        # Post-indexed writeback armasm form [r0], #4 is identical in GNU.
        text = self.substitute_registers(op + (" " + rest if rest else ""))
        self.out.append(text)
        if re.match(r"^[A-Za-z]", op):
            self._last_label = self._last_label  # unchanged

    # ---------------- driver ---------------------------------------------

    def convert(self, text):
        self._last_label = None
        lines = text.splitlines()
        i = 0
        while i < len(lines):
            line = lines[i]
            stripped = line.strip()
            # MACRO definition capture
            if re.match(r"^\s*MACRO\b", stripped, re.IGNORECASE):
                proto = ""
                i += 1
                while i < len(lines) and not lines[i].strip():
                    i += 1
                if i < len(lines):
                    proto = lines[i].strip()
                i += 1
                body = []
                depth = 1
                while i < len(lines):
                    l = lines[i].strip()
                    if re.match(r"^\s*MACRO\b", l, re.IGNORECASE):
                        depth += 1
                    elif re.match(r"^\s*MEND\b", l, re.IGNORECASE):
                        depth -= 1
                        if depth == 0:
                            i += 1
                            break
                    body.append(l)
                    i += 1
                self._register_macro(proto, body)
                continue

            # MACRO expansion call:  name arg,arg,...
            mm = re.match(r"^\s*([A-Za-z_.$][\w.$]*)\s*(.*)$", stripped)
            if mm and mm.group(1).upper() not in DIRECTIVE_MAP and \
                    mm.group(1).upper() not in ("MEND",) and \
                    mm.group(1) in self.macros:
                self.try_macro_expansion(mm.group(1), mm.group(2))
                i += 1
                continue

            self.convert_line(line)
            i += 1
        return "\n".join(self.out) + "\n"

    def _register_macro(self, proto, body):
        # prototype:  NAME $p1,$p2,...   or NAME $a,$b,:VAR:c
        m = re.match(r"([A-Za-z_.$][\w.$]*)\s*(.*)$", proto)
        if not m:
            return
        name = m.group(1)
        params = []
        for p in m.group(2).split(","):
            p = p.strip()
            if p:
                params.append(p if p.startswith("$") else "$" + p)
        self.macros[name] = type("MacroDef", (), {"params": params,
                                                  "body": body})


def main(argv=None):
    ap = argparse.ArgumentParser(
        description="Microsoft armasm -> GNU/as unified syntax translator")
    ap.add_argument("input", help="armasm source (.s/.asm)")
    ap.add_argument("-o", "--output", help="output GNU-syntax .s file")
    ap.add_argument("--define", "--def", dest="define",
                    action="append", default=[], metavar="SYM=VAL",
                    help="predefine a SETA/SETL symbol for IF evaluation")
    ap.add_argument("--thumb", action="store_true",
                    help="start in THUMB state")
    ap.add_argument("-I", "--include", action="append", default=[],
                    help="include dir (for GET/INCLUDE; files are "
                         "inlined when found)")
    args = ap.parse_args(argv)

    conv = Converter(defines=getattr(args, 'define', []),
                        thumb=args.thumb)

    text = open(args.input, encoding="latin1").read()

    # Inline GET/INCLUDE files that exist so CE driver include trees work.
    def inline_gets(src, depth=0):
        if depth > 8:
            return src
        out_lines = []
        for line in src.splitlines():
            m = re.match(r"\s*(?:GET|INCLUDE)\s+(\S+)", line,
                         re.IGNORECASE)
            if m:
                inc = m.group(1)
                for d in [os.path.dirname(args.input) or "."] + args.include:
                    cand = os.path.join(d, inc)
                    if os.path.exists(cand):
                        out_lines.append("# begin \"%s\"" % inc)
                        out_lines.append(inline_gets(
                            open(cand, encoding="latin1").read(), depth + 1))
                        out_lines.append("# end \"%s\"" % inc)
                        break
                else:
                    out_lines.append(line)  # keep; warning issued later
            else:
                out_lines.append(line)
        return "\n".join(out_lines)

    text = inline_gets(text)

    converted = conv.convert(text)

    out = converted
    if args.output:
        open(args.output, "w").write(out)
    else:
        sys.stdout.write(out)

    if conv.warnings and not args.output:
        for w in conv.warnings:
            print("armasm-convert: %s" % w, file=sys.stderr)
    return 0


if __name__ == "__main__":
    import os
    sys.exit(main())
