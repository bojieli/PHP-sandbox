/*
  +----------------------------------------------------------------------+
  | PHP version 4.0                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2001 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Sascha Schumann <sascha@schumann.cx>                        |
  +----------------------------------------------------------------------+
*/

#include "php.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "php_ini.h"
#include "php_globals.h"
#define STATE_TAG SOME_OTHER_STATE_TAG
#include "basic_functions.h"
#undef STATE_TAG

#define url_adapt_ext url_adapt_ext_ex
#define url_scanner url_scanner_ex

#include "php_smart_str.h"

static PHP_INI_MH(OnUpdateTags)
{
	url_adapt_state_ex_t *ctx;
	char *key;
	char *lasts;
	char *tmp;
	
	ctx = &BG(url_adapt_state_ex);
	
	tmp = estrndup(new_value, new_value_length);
	
	if (ctx->tags)
		zend_hash_destroy(ctx->tags);
	else
		ctx->tags = malloc(sizeof(HashTable));
	
	zend_hash_init(ctx->tags, 0, NULL, NULL, 1);
	
	for (key = php_strtok_r(tmp, ",", &lasts);
			key;
			key = php_strtok_r(NULL, ",", &lasts)) {
		char *val;

		val = strchr(key, '=');
		if (val) {
			char *q;
			int keylen;
			
			*val++ = '\0';
			for (q = key; *q; q++)
				*q = tolower(*q);
			keylen = q - key;
			/* key is stored withOUT NUL
			   val is stored WITH    NUL */
			zend_hash_add(ctx->tags, key, keylen, val, strlen(val)+1, NULL);
		}
	}

	efree(tmp);

	return SUCCESS;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("url_rewriter.tags", "a=href,area=href,frame=src,form=fakeentry", PHP_INI_ALL, OnUpdateTags, url_adapt_state_ex, php_basic_globals, basic_globals)
PHP_INI_END()

/*!re2c
any = [\000-\377];
N = (any\[<]);
alpha = [a-zA-Z];
*/

#define YYFILL(n) goto done
#define YYCTYPE unsigned char
#define YYCURSOR p
#define YYLIMIT q
#define YYMARKER r
	
static inline void append_modified_url(smart_str *url, smart_str *dest, smart_str *name, smart_str *val, const char *separator)
{
	register const char *p, *q, *r;
	const char *bash = NULL;
	const char *sep = "?";
	
	q = (p = url->c) + url->len;

scan:
/*!re2c
  ":"		{ smart_str_append(dest, url); return; }
  "?"		{ sep = separator; goto done; }
  "#"		{ bash = p - 1; goto done; }
  (any\[:?#])+		{ goto scan; }
*/
done:
	
	/* Don't modify URLs of the format "#mark" */
	if (bash && bash - url->c == 0) {
		smart_str_append(dest, url);
		return;
	}

	if (bash)
		smart_str_appendl(dest, url->c, bash - url->c);
	else
		smart_str_append(dest, url);

	smart_str_appends(dest, sep);
	smart_str_append(dest, name);
	smart_str_appendc(dest, '=');
	smart_str_append(dest, val);

	if (bash)
		smart_str_appendl(dest, bash, q - bash);
}


#undef YYFILL
#undef YYCTYPE
#undef YYCURSOR
#undef YYLIMIT
#undef YYMARKER

static inline void tag_arg(url_adapt_state_ex_t *ctx, char quote TSRMLS_DC)
{
	char f = 0;

	if (strncasecmp(ctx->arg.c, ctx->lookup_data, ctx->arg.len) == 0)
		f = 1;

	smart_str_appendc(&ctx->result, quote);
	if (f) {
		append_modified_url(&ctx->val, &ctx->result, &ctx->q_name, &ctx->q_value, PG(arg_separator).output);
	} else {
		smart_str_append(&ctx->result, &ctx->val);
	}
	smart_str_appendc(&ctx->result, quote);
}

enum {
	STATE_PLAIN = 0,
	STATE_TAG,
	STATE_NEXT_ARG,
	STATE_ARG,
	STATE_BEFORE_VAL,
	STATE_VAL
};

#define YYFILL(n) goto stop
#define YYCTYPE unsigned char
#define YYCURSOR xp
#define YYLIMIT end
#define YYMARKER q
#define STATE ctx->state

#define STD_PARA url_adapt_state_ex_t *ctx, char *start, char *YYCURSOR TSRMLS_DC
#define STD_ARGS ctx, start, xp TSRMLS_CC

static inline void passthru(STD_PARA) 
{
	smart_str_appendl(&ctx->result, start, YYCURSOR - start);
}

static inline void handle_form(STD_PARA) 
{
	if (ctx->tag.len == 4 && strncasecmp(ctx->tag.c, "form", 4) == 0) {
		smart_str_appends(&ctx->result, "<input type=\"hidden\" name=\""); 
		smart_str_append(&ctx->result, &ctx->q_name);
		smart_str_appends(&ctx->result, "\" value=\"");
		smart_str_append(&ctx->result, &ctx->q_value);
		smart_str_appends(&ctx->result, "\" />");
	}
}

/*
 *  HANDLE_TAG copies the HTML Tag and checks whether we 
 *  have that tag in our table. If we might modify it,
 *  we continue to scan the tag, otherwise we simply copy the complete
 *  HTML stuff to the result buffer.
 */

static inline void handle_tag(STD_PARA) 
{
	int ok = 0;
	int i;

	ctx->tag.len = 0;
	smart_str_appendl(&ctx->tag, start, YYCURSOR - start);
	for (i = 0; i < ctx->tag.len; i++)
		ctx->tag.c[i] = tolower(ctx->tag.c[i]);
	if (zend_hash_find(ctx->tags, ctx->tag.c, ctx->tag.len, (void **) &ctx->lookup_data) == SUCCESS)
		ok = 1;
	STATE = ok ? STATE_NEXT_ARG : STATE_PLAIN;
}

static inline void handle_arg(STD_PARA) 
{
	ctx->arg.len = 0;
	smart_str_appendl(&ctx->arg, start, YYCURSOR - start);
}

static inline void handle_val(STD_PARA, char quotes, char type) 
{
	smart_str_setl(&ctx->val, start + quotes, YYCURSOR - start - quotes * 2);
	tag_arg(ctx, type TSRMLS_CC);
}

#ifdef SCANNER_DEBUG
#define scdebug(x) printf x
#else
#define scdebug(x)
#endif

static inline void mainloop(url_adapt_state_ex_t *ctx, const char *newdata, size_t newlen TSRMLS_DC)
{
	char *end, *q;
	char *xp;
	char *start;
	int rest;

	smart_str_appendl(&ctx->buf, newdata, newlen);
	
	YYCURSOR = ctx->buf.c;
	YYLIMIT = ctx->buf.c + ctx->buf.len;

	switch (STATE) {
		case STATE_PLAIN: goto state_plain;
		case STATE_TAG: goto state_tag;
		case STATE_NEXT_ARG: goto state_next_arg;
		case STATE_ARG: goto state_arg;
		case STATE_BEFORE_VAL: goto state_before_val;
		case STATE_VAL: goto state_val;
	}
	

state_plain_begin:
	STATE = STATE_PLAIN;
	
state_plain:
	start = YYCURSOR;
/*!re2c
  "<"				{ passthru(STD_ARGS); STATE = STATE_TAG; goto state_tag; }
  N+ 				{ passthru(STD_ARGS); goto state_plain; }
*/

state_tag:	
	start = YYCURSOR;
/*!re2c
  alpha+	{ handle_tag(STD_ARGS); /* Sets STATE */; passthru(STD_ARGS); if (STATE == STATE_PLAIN) goto state_plain; else goto state_next_arg; }
  any		{ passthru(STD_ARGS); goto state_plain_begin; }
*/

state_next_arg_begin:
	STATE = STATE_NEXT_ARG;
	
state_next_arg:
	start = YYCURSOR;
/*!re2c
  ">"		{ passthru(STD_ARGS); handle_form(STD_ARGS); goto state_plain_begin; }
  [ \v\t\n]+	{ passthru(STD_ARGS); goto state_next_arg; }
  alpha		{ --YYCURSOR; STATE = STATE_ARG; goto state_arg; }
  any		{ passthru(STD_ARGS); goto state_plain_begin; }
*/

state_arg:
	start = YYCURSOR;
/*!re2c
  alpha+	{ passthru(STD_ARGS); handle_arg(STD_ARGS); STATE = STATE_BEFORE_VAL; goto state_before_val; }
  any		{ passthru(STD_ARGS); STATE = STATE_NEXT_ARG; goto state_next_arg; }
*/

state_before_val:
	start = YYCURSOR;
/*!re2c
  [ ]* "=" [ ]*		{ passthru(STD_ARGS); STATE = STATE_VAL; goto state_val; }
  any				{ --YYCURSOR; goto state_next_arg_begin; }
*/


state_val:
	start = YYCURSOR;
/*!re2c
  ["] (any\[">])* ["]	{ handle_val(STD_ARGS, 1, '"'); goto state_next_arg_begin; }
  ['] (any\['>])* [']	{ handle_val(STD_ARGS, 1, '\''); goto state_next_arg_begin; }
  (any\[ \n>"'])+		{ handle_val(STD_ARGS, 0, '"'); goto state_next_arg_begin; }
  any					{ passthru(STD_ARGS); goto state_next_arg_begin; }
*/

stop:
	rest = YYLIMIT - start;
	scdebug(("stopped in state %d at pos %d (%d:%c) %d\n", STATE, YYCURSOR - ctx->buf.c, *YYCURSOR, *YYCURSOR, rest));
	/* XXX: Crash avoidance. Need to work with reporter to figure out what goes wrong */	
	if (rest < 0) rest = 0;
	
	if (rest) memmove(ctx->buf.c, start, rest);
	ctx->buf.len = rest;
}


char *url_adapt_single_url(const char *url, size_t urllen, const char *name, const char *value, size_t *newlen TSRMLS_DC)
{
	smart_str surl = {0};
	smart_str buf = {0};
	smart_str sname = {0};
	smart_str sval = {0};

	smart_str_setl(&surl, url, urllen);
	smart_str_sets(&sname, name);
	smart_str_sets(&sval, value);

	append_modified_url(&surl, &buf, &sname, &sval, PG(arg_separator).output);

	smart_str_0(&buf);
	if (newlen) *newlen = buf.len;
	
	return buf.c;
}

char *url_adapt_ext(const char *src, size_t srclen, const char *name, const char *value, size_t *newlen, zend_bool do_flush TSRMLS_DC)
{
	char *ret;
	url_adapt_state_ex_t *ctx;
	char *retval;

	ctx = &BG(url_adapt_state_ex);

	smart_str_sets(&ctx->q_name, name);
	smart_str_sets(&ctx->q_value, value);
	mainloop(ctx, src, srclen TSRMLS_CC);

	*newlen = ctx->result.len;
	if (!ctx->result.c) {
		smart_str_appendl(&ctx->result, "", 0);
	}
	smart_str_0(&ctx->result);
	if (do_flush) {
		smart_str_appendl(&ctx->result, ctx->buf.c, ctx->buf.len);
		*newlen += ctx->buf.len;
		smart_str_free(&ctx->buf);
	}
	retval = ctx->result.c;
	ctx->result.c = NULL;
	ctx->result.len = 0;
	return retval;
}

int php_url_scanner_ex_activate(TSRMLS_D)
{
	url_adapt_state_ex_t *ctx;
	
	ctx = &BG(url_adapt_state_ex);

	memset(ctx, 0, ((size_t) &((url_adapt_state_ex_t *)0)->tags));

	return SUCCESS;
}

int php_url_scanner_ex_deactivate(TSRMLS_D)
{
	url_adapt_state_ex_t *ctx;
	
	ctx = &BG(url_adapt_state_ex);

	smart_str_free(&ctx->result);
	smart_str_free(&ctx->buf);
	smart_str_free(&ctx->tag);
	smart_str_free(&ctx->arg);

	return SUCCESS;
}

PHP_MINIT_FUNCTION(url_scanner)
{
	url_adapt_state_ex_t *ctx;
	
	ctx = &BG(url_adapt_state_ex);

	ctx->tags = NULL;
	
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(url_scanner)
{
	UNREGISTER_INI_ENTRIES();
	zend_hash_destroy(BG(url_adapt_state_ex).tags);
	free(BG(url_adapt_state_ex).tags);
	return SUCCESS;
}
