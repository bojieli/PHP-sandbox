#ifndef CONVERSION_H
#define CONVERSION_H

BEGIN_EXTERN_C()

extern void php_pval_to_variant(pval *pval_arg, VARIANT *var_arg);
extern void php_pval_to_variant_ex(pval *pval_arg, VARIANT *var_arg, pval *pval_type);
extern void php_variant_to_pval(VARIANT *var_arg, pval *pval_arg, int persistent);
extern OLECHAR *php_char_to_OLECHAR(char *C_str, uint strlen);
extern char *php_OLECHAR_to_char(OLECHAR *unicode_str, uint *out_length, int persistent);

END_EXTERN_C()

#endif