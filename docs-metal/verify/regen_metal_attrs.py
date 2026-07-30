#!/usr/bin/env python3
"""Regenerate the Metal section of Attr.td from a clang-tblgen --dump-json dump.

Used to reconstruct the definitions after they were lost, and kept because it
documents exactly what each attribute is made of. The dump is authoritative:
it is what TableGen itself parsed.

Usage: regen_metal_attrs.py <attr.json> > metal_attrs.td
"""
import json, sys

d = json.load(open(sys.argv[1]))


def deref(x):
    if isinstance(x, dict) and "def" in x:
        return d.get(x["def"], x)
    return x


def spelling(sp):
    s = deref(sp)
    v, n = s.get("Variety"), s.get("Name")
    ns = s.get("Namespace") or ""
    ver = s.get("Version", 1)
    if v == "CXX11":
        return 'CXX11<"%s", "%s"%s>' % (ns, n, "" if ver == 1 else ", %d" % ver)
    if v == "C2x":
        return 'C2x<"%s", "%s">' % (ns, n)
    if v in ("GNU", "Keyword", "Declspec", "Microsoft", "Pragma", "GCC", "Clang"):
        return '%s<"%s">' % (v, n)
    raise SystemExit("unknown variety %r" % v)


ARG_KIND = {
    "ExprArgument": lambda a: 'ExprArgument<"%s"%s>' % (a["Name"], ", 1" if a.get("Optional") else ""),
    "StringArgument": lambda a: 'StringArgument<"%s"%s>' % (a["Name"], ", 1" if a.get("Optional") else ""),
    "IdentifierArgument": lambda a: 'IdentifierArgument<"%s"%s>' % (a["Name"], ", 1" if a.get("Optional") else ""),
    "TypeArgument": lambda a: 'TypeArgument<"%s"%s>' % (a["Name"], ", 1" if a.get("Optional") else ""),
    "IntArgument": lambda a: 'IntArgument<"%s"%s>' % (a["Name"], ", 1" if a.get("Optional") else ""),
    "UnsignedArgument": lambda a: 'UnsignedArgument<"%s"%s>' % (a["Name"], ", 1" if a.get("Optional") else ""),
    "BoolArgument": lambda a: 'BoolArgument<"%s"%s>' % (a["Name"], ", 1" if a.get("Optional") else ""),
    "VariadicExprArgument": lambda a: 'VariadicExprArgument<"%s">' % a["Name"],
    "VariadicUnsignedArgument": lambda a: 'VariadicUnsignedArgument<"%s">' % a["Name"],
    "VariadicStringArgument": lambda a: 'VariadicStringArgument<"%s">' % a["Name"],
    "VariadicIdentifierArgument": lambda a: 'VariadicIdentifierArgument<"%s">' % a["Name"],
    "EnumArgument": lambda a: 'EnumArgument<"%s", "%s", [%s], [%s]%s>' % (
        a["Name"], a["Type"],
        ", ".join('"%s"' % v for v in a.get("Values", [])),
        ", ".join('"%s"' % v for v in a.get("Enums", [])),
        ", 1" if a.get("Optional") else ""),
}


def argument(a):
    r = deref(a)
    for k in r.get("!superclasses", [])[::-1]:
        if k in ARG_KIND:
            return ARG_KIND[k](r)
    raise SystemExit("unknown arg %r" % r.get("!superclasses"))


def subjects(sub):
    if not sub:
        return None
    r = deref(sub)
    names = [deref(x).get("!name") or x.get("printable") for x in r.get("Subjects", [])]
    names = [n.split("::")[-1] for n in names if n]
    diag = r.get("Diag")
    extra = ""
    if diag and deref(diag).get("!name") == "ErrorDiag":
        extra = ", ErrorDiag"
    return "SubjectList<[%s]%s>" % (", ".join(names), extra)


out = []
for name in sorted(a for a in d["!instanceof"].get("Attr", []) if a.startswith("Metal")):
    r = d[name]
    supers = r.get("!superclasses", [])
    base = "InheritableParamAttr" if "InheritableParamAttr" in supers else \
           "TypeAttr" if "TypeAttr" in supers else \
           "StmtAttr" if "StmtAttr" in supers else \
           "InheritableAttr" if "InheritableAttr" in supers else "Attr"
    lines = ["def %s : %s {" % (name, base)]
    lines.append("  let Spellings = [%s];" %
                 ", ".join(spelling(s) for s in r.get("Spellings", [])))
    args = r.get("Args") or []
    if args:
        lines.append("  let Args = [%s];" % ", ".join(argument(a) for a in args))
    sl = subjects(r.get("Subjects"))
    if sl:
        lines.append("  let Subjects = %s;" % sl)
    if r.get("LangOpts"):
        lines.append("  let LangOpts = [%s];" %
                     ", ".join(deref(x).get("!name", x.get("printable")) for x in r["LangOpts"]))
    if r.get("ASTNode") is False or r.get("ASTNode") == 0:
        lines.append("  let ASTNode = 0;")
    if r.get("PragmaAttributeSupport") is False or r.get("PragmaAttributeSupport") == 0:
        lines.append("  let PragmaAttributeSupport = 0;")
    am = r.get("AdditionalMembers")
    if am:
        body = am if isinstance(am, str) else am.get("printable", "")
        if body.strip():
            lines.append("  let AdditionalMembers = [{%s}];" % body)
    lines.append("  let Documentation = [Undocumented];")
    lines.append("}")
    out.append("\n".join(lines))

print("\n\n".join(out))
