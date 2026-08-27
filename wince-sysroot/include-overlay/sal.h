/*
 * sal.h - Microsoft Source-Code Annotation Language, standard edition.
 *
 * Windows CE SDK-era headers do not annotate (the vendored specstrings.h
 * carries the old __in/__out spellings), but modern sources ported from
 * the desktop SDK use the _In_/_Out_ family.  This header provides the
 * annotation macros as conforming no-op spellings (the analysis-tier
 * macros are recognized but inert without a static analyzer), matching
 * what the MSVC sal.h provides in analysis=off mode.
 */
#ifndef __SAL_H__
#define __SAL_H__

#if defined(_MSC_VER) && _MSC_VER >= 1700
#include_next <sal.h>
#else

/* Annotation-projection macros: all no-ops for compilation.  */
#define _In_
#define _In_opt_
#define _In_z_
#define _In_opt_z_
#define _In_reads_(s)
#define _In_reads_opt_(s)
#define _In_reads_bytes_(s)
#define _In_reads_bytes_opt_(s)
#define _Out_
#define _Out_opt_
#define _Out_writes_(s)
#define _Out_writes_opt_(s)
#define _Out_writes_bytes_(s)
#define _Out_writes_bytes_opt_(s)
#define _Out_writes_to_(s,c)
#define _Out_writes_to_opt_(s,c)
#define _Inout_
#define _Inout_opt_
#define _Inout_z_
#define _Inout_updates_(s)
#define _Inout_updates_opt_(s)
#define _Inout_updates_bytes_(s)
#define _Inout_updates_bytes_opt_(s)
#define _Ret_
#define _Ret_opt_
#define _Ret_maybenull_
#define _Ret_notnull_
#define _Deref_out_
#define _Deref_out_opt_
#define _Deref_out_range_(l,h)
#define _Post_
#define _Post_ptr_invalid_
#define _Pre_
#define _Pre_opt_
#define _Pre_valid_
#define _Pre_notnull_
#define _Maybenull_
#define _Notnull_
#define _Null_
#define _Success_(e)
#define _Return_type_success_(e)
#define _Check_return_
#define _Must_inspect_result_
#define _Use_decl_annotations_
#define _Printf_format_string_
#define _Scanf_format_string_
#define _Printf_format_string_params_(x)
#define _Field_size_(s)
#define _Field_size_opt_(s)
#define _Field_size_bytes_(s)
#define _Field_size_bytes_opt_(s)
#define _Struct_size_bytes_(s)
#define _Analysis_assume_(e)
#define _Analysis_assume_nullterminates_(e)
#define _Analysis_noreturn_
#define _In_range_(l,h)
#define _Out_range_(l,h)
#define _In_typedef_(e)
#define _Out_typedef_(e)
#define _Inout_typedef_(e)
#define _Ret_typedef_(e)
#define _Function_class_(n)
#define _Literal_
#define _Notliteral_
#define _Interlocked_operand_
#define _On_failure_(a)
#define _On_success_(a)
#define _Group_(a)

/* Old-style SAL (kept defined for mixed-era sources).  */
#ifndef __SAL_H_OLD__
#define __SAL_H_OLD__
#define __in
#define __in_opt
#define __in_z
#define __in_ecount(s)
#define __in_bcount(s)
#define __out
#define __out_opt
#define __out_z
#define __out_ecount(s)
#define __out_bcount(s)
#define __inout
#define __inout_opt
#define __inout_ecount(s)
#define __inout_bcount(s)
#define __deref_in
#define __deref_out
#define __deref_inout
#define __ecount(s)
#define __bcount(s)
#define __parts(s)
#define __valid
#define __readonly
#define __nullterminated
#define __nullnullterminated
#define __maybenull
#endif /* __SAL_H_OLD__ */

#endif /* _MSC_VER >= 1700 */
#endif /* __SAL_H__ */
