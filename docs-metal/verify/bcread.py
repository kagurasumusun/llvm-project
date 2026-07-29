"""Minimal LLVM bitstream reader, enough to inspect the TYPE block.

Written from the LLVM Bitstream Format specification so that claims about
Apple's AIR bitcode can be checked against the bytes rather than taken on
faith from a summary document.
"""
import struct, sys

END_BLOCK, ENTER_SUBBLOCK, DEFINE_ABBREV, UNABBREV_RECORD = 0, 1, 2, 3
FIRST_APP_ABBREV = 4
BLOCKINFO_BLOCK_ID = 0

# llvm/Bitcode/LLVMBitCodes.h
BLOCK_NAMES = {0:'BLOCKINFO',8:'MODULE',9:'PARAMATTR',10:'PARAMATTR_GROUP',
 11:'CONSTANTS',12:'FUNCTION',13:'IDENTIFICATION',14:'VALUE_SYMTAB',
 15:'METADATA',16:'METADATA_ATTACHMENT',17:'TYPE',18:'USELIST',
 19:'MODULE_STRTAB',20:'GLOBALVAL_SUMMARY',21:'OPERAND_BUNDLE_TAGS',
 22:'METADATA_KIND',23:'STRTAB',24:'FULL_LTO_GLOBALVAL_SUMMARY',
 25:'SYMTAB',26:'SYNC_SCOPE_NAMES'}

TYPE_CODES = {1:'NUMENTRY',2:'VOID',3:'FLOAT',4:'DOUBLE',5:'LABEL',6:'OPAQUE',
 7:'INTEGER',8:'POINTER',9:'FUNCTION_OLD',10:'HALF',11:'ARRAY',12:'VECTOR',
 13:'X86_FP80',14:'FP128',15:'PPC_FP128',16:'METADATA',17:'X86_MMX',
 18:'STRUCT_ANON',19:'STRUCT_NAME',20:'STRUCT_NAMED',21:'FUNCTION',
 22:'TOKEN',23:'BFLOAT',24:'X86_AMX',25:'OPAQUE_POINTER',26:'TARGET_TYPE'}

class Reader:
    def __init__(self, data, bitpos=0):
        self.d = data; self.pos = bitpos
    def read(self, n):
        v = 0
        for i in range(n):
            byte = self.d[self.pos >> 3]
            bit = (byte >> (self.pos & 7)) & 1
            v |= bit << i
            self.pos += 1
        return v
    def vbr(self, n):
        piece = self.read(n); hi = 1 << (n-1)
        if not (piece & hi): return piece
        result = piece & (hi-1); shift = n-1
        while True:
            piece = self.read(n)
            result |= (piece & (hi-1)) << shift
            if not (piece & hi): return result
            shift += n-1
    def align32(self):
        self.pos = (self.pos + 31) & ~31
    def at_end(self):
        return self.pos >= len(self.d)*8

class Abbrev:
    def __init__(self, ops): self.ops = ops

def parse_abbrev(r):
    numops = r.vbr(5); ops=[]
    for _ in range(numops):
        if r.read(1):
            ops.append(('lit', r.vbr(8)))
        else:
            enc = r.read(3)
            if enc in (1,2): ops.append(('enc', enc, r.vbr(5)))
            else: ops.append(('enc', enc, None))
    return Abbrev(ops)

def read_abbrev_op(r, op):
    kind = op[0]
    if kind=='lit': return op[1]
    enc = op[1]
    if enc==1: return r.read(op[2]) if op[2] else 0
    if enc==2: return r.vbr(op[2])
    if enc==4:  # char6
        v=r.read(6)
        return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._"[v]
    raise ValueError('unexpected encoding %r'%(op,))

def parse_block(r, blockid, abbrev_width, blockinfo, out, depth=0):
    abbrevs = list(blockinfo.get(blockid, []))
    cur_bid = None
    while True:
        if r.at_end(): return
        code = r.read(abbrev_width)
        if code == END_BLOCK:
            r.align32(); return
        if code == ENTER_SUBBLOCK:
            bid = r.vbr(8); newwidth = r.vbr(4); r.align32()
            length = r.read(32)
            endpos = r.pos + length*32
            parse_block(r, bid, newwidth, blockinfo, out, depth+1)
            r.pos = endpos
            continue
        if code == DEFINE_ABBREV:
            a = parse_abbrev(r)
            if blockid == BLOCKINFO_BLOCK_ID and cur_bid is not None:
                blockinfo.setdefault(cur_bid, []).append(a)
            else:
                abbrevs.append(a)
            continue
        if code == UNABBREV_RECORD:
            rc = r.vbr(6); n = r.vbr(6)
            vals = [r.vbr(6) for _ in range(n)]
        else:
            idx = code - FIRST_APP_ABBREV
            if idx >= len(abbrevs): raise ValueError('bad abbrev %d in block %d'%(code,blockid))
            a = abbrevs[idx]; vals=[]; rc=None
            i=0
            while i < len(a.ops):
                op = a.ops[i]
                if op[0]=='enc' and op[1]==3:      # array
                    cnt = r.vbr(6); elt = a.ops[i+1]; i+=2
                    arr=[read_abbrev_op(r, elt) for _ in range(cnt)]
                    if arr and isinstance(arr[0],str): vals.append(''.join(arr))
                    else: vals.extend(arr)
                    continue
                if op[0]=='enc' and op[1]==5:      # blob
                    cnt = r.vbr(6); r.align32()
                    blob = r.d[r.pos>>3 : (r.pos>>3)+cnt]
                    r.pos += cnt*8; r.align32()
                    vals.append(blob); i+=1; continue
                v = read_abbrev_op(r, op)
                if rc is None: rc = v
                else: vals.append(v)
                i+=1
        if blockid == BLOCKINFO_BLOCK_ID and rc == 1 and vals:
            cur_bid = vals[0]
        out.append((blockid, rc, vals))

def load(path):
    d = open(path,'rb').read()
    if d[:4] == bytes.fromhex('dec0170b'):
        off, size = struct.unpack_from('<II', d, 8)
        d = d[off:off+size]
    if d[:4] != b'BC\xc0\xde':
        raise ValueError('not bitcode: %s' % d[:4].hex())
    r = Reader(d, 32)
    out=[]; blockinfo={}
    while not r.at_end():
        try:
            code = r.read(2)
        except IndexError:
            break
        if code != ENTER_SUBBLOCK: break
        bid = r.vbr(8); w = r.vbr(4); r.align32()
        length = r.read(32); endpos = r.pos + length*32
        parse_block(r, bid, w, blockinfo, out)
        r.pos = endpos
    return out
