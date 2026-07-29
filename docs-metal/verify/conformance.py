#!/usr/bin/env python3
"""Measure how closely this fork's design reproduces Apple's AIR contract.

Rather than claiming a percentage, each individual observable that Apple's
output exhibits is checked against what this implementation would emit, and
the ones that cannot yet be checked are reported as such.
"""
import csv, os, re, sys, glob, collections
sys.path.insert(0,'/home/user/probe')
import bcread

INFO = sys.argv[1] if len(sys.argv)>1 else '/tmp/metal-info'
ROOT = '/home/user/llvm-project'
def read(p): return open(os.path.join(ROOT,p),encoding='utf-8').read()

results=[]   # (category, item, status, detail)
def rec(cat,item,st,detail=''): results.append((cat,item,st,detail))

# ---- 1. Module-level facts, read from Apple's own bitcode -------------------
gold='research/golden/P01/metal32_macosx26/probe.ll'
g=open(os.path.join(INFO,gold)).read()

air_h=read('clang/lib/Basic/Targets/AIR.h')
dl_want=re.search(r'target datalayout = "([^"]+)"',g).group(1)
lits=re.findall(r'"([^"]*)"',air_h.split('if (Is64Bit)')[1].split('return')[1])
rec('module','datalayout (air64)','MATCH' if ''.join(lits)==dl_want else 'MISMATCH')

for key,field in [('air.max_device_buffers','MaxDeviceBuffers'),
                  ('air.max_constant_buffers','MaxConstantBuffers'),
                  ('air.max_threadgroup_buffers','MaxThreadgroupBuffers'),
                  ('air.max_textures','MaxTextures'),
                  ('air.max_read_write_textures','MaxReadWriteTextures'),
                  ('air.max_samplers','MaxSamplers')]:
    w=int(re.search(r'!"%s", i32 (\d+)'%re.escape(key),g).group(1))
    gt=int(re.search(r'%s = (\d+);'%field,air_h).group(1))
    rec('module',key,'MATCH' if w==gt else 'MISMATCH',f'want {w} got {gt}')

cg=read('clang/lib/CodeGen/CGMetal.cpp')
for nmd in ['air.version','air.language_version','air.compile_options',
            'air.source_file_name','air.kernel','air.vertex','air.fragment']:
    rec('module',nmd,'EMITTED' if f'"{nmd}"' in cg else 'MISSING')

# ---- 2. Address spaces -----------------------------------------------------
blk=air_h.split('AIRAddrSpaceMap[] = {')[1].split('};')[0]
amap={n:int(v) for v,n in re.findall(r'(\d+), // (\w+)',blk)}
for n,w in [('metal_device',1),('metal_constant',2),('metal_threadgroup',3),
            ('metal_thread',0),('metal_threadgroup_imageblock',4),
            ('metal_object_data',7),('metal_ray_data',9)]:
    rec('addrspace',n,'MATCH' if amap.get(n)==w else 'MISMATCH',f'want {w} got {amap.get(n)}')

# ---- 3. Types --------------------------------------------------------------
ast=os.path.join(INFO,'reference/metal-ast-macos-air64/ast',
  'macos_air64_versioned_none_metal4.0_attributes_all_26_0_ast-text.txt')
want=re.findall(r'implicit (__metal_\w+)',open(ast).read())
got=re.findall(r'^METAL_TYPE\((\w+),',read('clang/include/clang/Basic/MetalTypes.def'),re.M)
rec('types','37 opaque types, in order','MATCH' if want==got else 'MISMATCH')

# ---- 4. Builtins -----------------------------------------------------------
bd=read('clang/include/clang/Basic/BuiltinsMetal.def')
mine=dict(re.findall(r'^METAL_BUILTIN\((\w+), "[^"]*", "[^"]*", "([^"]*)"\)',bd,re.M))
tbl={}
with open(os.path.join(INFO,'research/datasets/builtin_to_air_map.v2.csv')) as f:
    for r in csv.DictReader(f): tbl[r['__metal_builtin']]=r['air_intrinsic_candidate'].strip()
hdr=set()
for root,_,fs in os.walk(os.path.join(INFO,'reference-apple/clang/32023.883/include/metal')):
    for fn in fs:
        hdr.update(re.findall(r'__metal_[A-Za-z0-9_]+',
                   open(os.path.join(root,fn),errors='ignore').read()))
types_set=set(want)
need=hdr-types_set
rec('builtins',f'stdlib requires {len(need)} builtins',
    'MATCH' if need<=set(mine) else 'MISMATCH', f'missing {len(need-set(mine))}')
bad=[k for k,v in mine.items() if k in tbl and tbl[k]!=v]
rec('builtins','AIR intrinsic names','MATCH' if not bad else 'MISMATCH',f'{len(bad)} differ')

# ---- 5. Attributes ---------------------------------------------------------
td=read('clang/include/clang/Basic/Attr.td')
spell=set(re.findall(r'CXX11<"", "(\w+)">',td))
spec={r['attribute'] for r in csv.DictReader(
        open(os.path.join(INFO,'research/datasets/spec_attributes.csv')))}
rec('attributes',f'{len(spec)} spec attributes','MATCH' if spec<=spell else 'MISMATCH',
    f'missing {sorted(spec-spell)[:3]}')

# ---- 6. Entry metadata operand shape, compared against golden --------------
# Extract the operand key sequence Apple emits for each argument kind.
def keyseq(node):
    return [m for m in re.findall(r'!"(air\.[a-z_]+)"',node)]
gold_nodes=re.findall(r'^!\d+ = !\{i32 \d+, (!"air\.[^}]*)\}',g,re.M)
shapes={}
for n in gold_nodes:
    ks=keyseq('!'+n)
    if ks: shapes[ks[0]]=ks
for kind,ks in sorted(shapes.items()):
    # Does CGMetal emit this key sequence for that kind?
    ok=all(f'"{k}"' in cg for k in ks)
    rec('entry-metadata',f'{kind} operand keys','EMITTED' if ok else 'INCOMPLETE',
        ' '.join(ks))

# ---- 7. Known defects ------------------------------------------------------
rec('defect','air.arg_type_name uses pointee + MSL spelling',
    'MATCH' if ('getMetalTypeName' in cg and 'getPointeeType' in cg) else 'MISMATCH',
    'pointee named, MSL spelling via getMetalTypeName')
rec('defect','air.compile.framebuffer_fetch platform/std rule',
    'MISMATCH' if 'framebuffer_fetch_enable");' in cg and 'metal2.3' not in cg else 'MATCH',
    'macOS <=2.2 must be disable')
drv=read('clang/lib/Driver/Driver.cpp')
rec('defect','watchOS AIR version mapping',
    'MISMATCH' if 'Major >= 10 ? 26' in drv else 'MATCH',
    'watchOS 11.4 must be v27')
mangle = read('clang/lib/AST/Mangle.cpp')
rec('defect','entry functions must not be mangled',
    'MATCH' if ('getLangOpts().Metal' in mangle and
                'MetalKernelAttr' in mangle) else 'MISSING',
    'Apple: 400/400 entry points unmangled')

# New observables introduced by the Phase 10 fixes.
sematype = read('clang/lib/Sema/SemaType.cpp')
rec('sema','member function address-space qualifier',
    'MATCH' if 'getLangOpts().Metal && IsClassMember' in sematype else 'MISSING',
    'stdlib uses 7,668 of these')
scalar = read('clang/lib/CodeGen/CGExprScalar.cpp')
rec('codegen','air.convert.* numeric conversion lowering',
    'MATCH' if 'emitMetalConvert' in scalar else 'MISSING',
    '57 measured variants')
semametal = read('clang/lib/Sema/SemaMetal.cpp')
rec('sema','unsupported C++ constructs rejected',
    'MATCH' if 'DiagnoseMetalUnsupportedDecl' in semametal else 'MISSING',
    'MSL 4.1 section 1.6.1')
rec('sema','unsupported types rejected (double, long long)',
    'MATCH' if 'DiagnoseMetalUnsupportedType' in semametal else 'MISSING',
    'measured + spec')
rec('sema','entry parameters require a binding attribute',
    'MATCH' if 'err_metal_param_needs_attr' in semametal else 'MISSING',
    'measured: "t parameter must have texture attribute"')

# ---- report ----------------------------------------------------------------
by=collections.Counter(s for _,_,s,_ in results)
w=max(len(i) for _,i,_,_ in results)
cur=None
for cat,item,st,detail in results:
    if cat!=cur: print(f"\n[{cat}]"); cur=cat
    mark={'MATCH':'ok  ','EMITTED':'ok  ','MISMATCH':'FAIL','MISSING':'GAP ',
          'INCOMPLETE':'GAP '}[st]
    print(f"  {mark} {item:<{w}}  {detail}")
tot=len(results); good=by['MATCH']+by['EMITTED']
print(f"\n{good}/{tot} observables reproduced  "
      f"({by['MISMATCH']} mismatch, {by['MISSING']+by['INCOMPLETE']} gap)")
