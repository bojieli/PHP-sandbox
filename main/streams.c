/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2002 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors:                                                             |
   | Wez Furlong (wez@thebrainroom.com)                                   |
   | Borrowed code from:                                                  |
   | Rasmus Lerdorf <rasmus@lerdorf.on.ca>                                |
   | Jim Winstead <jimw@php.net>                                          |
   +----------------------------------------------------------------------+
 */

#define _GNU_SOURCE
#include "php.h"
#include "php_globals.h"
#include "php_network.h"
#include "php_open_temporary_file.h"

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif


#ifdef PHP_WIN32
#define EWOULDBLOCK WSAEWOULDBLOCK
#else
#include "build-defs.h"
#endif

static HashTable url_stream_wrappers_hash;

/* allocate a new stream for a particular ops */
PHPAPI php_stream *php_stream_alloc(php_stream_ops *ops, void *abstract, int persistent, const char *mode) /* {{{ */
{
	php_stream *ret;

	ret = (php_stream*) pemalloc(sizeof(php_stream), persistent);

	memset(ret, 0, sizeof(php_stream));

	ret->ops = ops;
	ret->abstract = abstract;
	ret->is_persistent = persistent;

	strlcpy(ret->mode, mode, sizeof(ret->mode));

	return ret;
}
/* }}} */

PHPAPI int php_stream_free(php_stream *stream, int call_dtor) /* {{{ */
{
	int ret = 1;

	php_stream_flush(stream);
	
	if (stream->wrapper && stream->wrapper->destroy) {
		stream->wrapper->destroy(stream);
	}

	if (call_dtor) {
		if (stream->fclose_stdiocast == PHP_STREAM_FCLOSE_FOPENCOOKIE) {
			/* calling fclose on an fopencookied stream will ultimately
				call this very same function.  If we were called via fclose,
				the cookie_closer unsets the fclose_stdiocast flags, so
				we can be sure that we only reach here when PHP code calls
				php_stream_free.
				Lets let the cookie code clean it all up.
			 */
			return fclose(stream->stdiocast);
		}
		
		php_stream_flush(stream);
		ret = stream->ops->close(stream);
	
		/* tidy up any FILE* that might have been fdopened */
		if (stream->fclose_stdiocast == PHP_STREAM_FCLOSE_FDOPEN && stream->stdiocast) {
			fclose(stream->stdiocast);
			stream->stdiocast = NULL;
		}
	}

	if (stream->wrapperdata) {
		FREE_ZVAL(stream->wrapperdata);
	}
	pefree(stream, stream->is_persistent);

	return ret;
}
/* }}} */

/* {{{ generic stream operations */
PHPAPI size_t php_stream_read(php_stream *stream, char *buf, size_t size)
{
	return stream->ops->read(stream, buf, size);
}

PHPAPI int php_stream_eof(php_stream *stream)
{
	/* we define our stream reading function so that it
	   must return EOF when an EOF condition occurs, when
	   working in unbuffered mode and called with these args */
	return stream->ops->read(stream, NULL, 0) == EOF ? 1 : 0;
}

PHPAPI int php_stream_putc(php_stream *stream, int c)
{
	unsigned char buf = c;

	if (php_stream_write(stream, &buf, 1) > 0) {
		return 1;
	}
	return EOF;
}

PHPAPI int php_stream_getc(php_stream *stream)
{
	char buf;

	if (php_stream_read(stream, &buf, 1) > 0) {
		return buf;
	}
	return EOF;
}

PHPAPI int php_stream_puts(php_stream *stream, char *buf)
{
	int len;
	char newline[2] = "\n"; /* is this OK for Win? */
	len = strlen(buf);

	if (len > 0 && php_stream_write(stream, buf, len) && php_stream_write(stream, newline, 1)) {
		return 1;
	}
	return 0;
}

PHPAPI char *php_stream_gets(php_stream *stream, char *buf, size_t maxlen)
{

	if (maxlen == 0) {
		buf[0] = 0;
		return buf;
	}

	if (stream->ops->gets) {
		return stream->ops->gets(stream, buf, maxlen);
	} else {
		/* unbuffered fgets - poor performance ! */
		size_t n = 0;
		char *c = buf;

		/* TODO: look at error returns? */

		while(n < maxlen && stream->ops->read(stream, c, 1) > 0) {
			n++;
			if (*c == '\n')	{
				c++;
				break;
			}
			c++;
		}
		*c = 0;
		return buf;
	}
}

PHPAPI int php_stream_flush(php_stream *stream)
{
	if (stream->ops->flush) {
		return stream->ops->flush(stream);
	}
	return 0;
}

PHPAPI size_t php_stream_write(php_stream *stream, const char *buf, size_t count)
{
	return stream->ops->write(stream, buf, count);
}

PHPAPI off_t php_stream_tell(php_stream *stream)
{
	off_t ret = -1;

	if (stream->ops->seek) {
		ret = stream->ops->seek(stream, 0, SEEK_CUR);
	}
	return ret;
}

PHPAPI int php_stream_seek(php_stream *stream, off_t offset, int whence)
{
	if (stream->ops->seek) {	
		return stream->ops->seek(stream, offset, whence);
	}

	/* emulate forward moving seeks with reads */
	if (whence == SEEK_CUR && offset > 0) {
		while(offset-- > 0) {
			if (php_stream_getc(stream) == EOF) {
				return -1;
			}
		}
		return 0;
	}
	
	zend_error(E_WARNING, "streams of type %s do not support seeking", stream->ops->label);
	return -1;
}

#define CHUNK_SIZE	8192

PHPAPI size_t php_stream_read_all(php_stream *src, char **buf, int persistent)
{
	size_t ret = 0;
	char *ptr;
	size_t len = 0, max_len;
	int step = CHUNK_SIZE;
	int min_room = CHUNK_SIZE / 4;
#if HAVE_MMAP
	int srcfd;
#endif

#if HAVE_MMAP
	/* try and optimize the case where we are copying from the start of a plain file.
	 * We could probably make this work in more situations, but I don't trust the stdio
	 * buffering layer.
	 * */
	if ( php_stream_is(src, PHP_STREAM_IS_STDIO) &&
			php_stream_tell(src) == 0 && 
			SUCCESS == php_stream_cast(src, PHP_STREAM_AS_FD, (void**)&srcfd, 0))
	{
		struct stat sbuf;

		if (fstat(srcfd, &sbuf) == 0) {
			void *srcfile;
			
			srcfile = mmap(NULL, sbuf.st_size, PROT_READ, MAP_SHARED, srcfd, 0);
			if (srcfile != (void*)MAP_FAILED) {

				*buf = pemalloc(persistent, sbuf.st_size);

				if (*buf)	{
					memcpy(*buf, srcfile, sbuf.st_size);
					ret = sbuf.st_size;
				}
				
				munmap(srcfile, sbuf.st_size);
				return ret;
			}
		}
		/* fall through - we might be able to copy in smaller chunks */
	}
#endif
	
	ptr = *buf = pemalloc(persistent, step);
	max_len = step;

	while((ret = php_stream_read(src, ptr, max_len - len)))	{
		len += ret;
		if (len + min_room >= max_len) {
			*buf = perealloc(*buf, max_len + step, persistent);
			max_len += step;
			ptr = *buf + len;
		}
	}
	if (len) {
		*buf = perealloc(*buf, len, persistent);
	} else {
		pefree(*buf, persistent);
		*buf = NULL;
	}
	return len;
}

PHPAPI size_t php_stream_copy_to_stream(php_stream *src, php_stream *dest, size_t maxlen)
{
	char buf[CHUNK_SIZE];
	size_t readchunk;
	size_t haveread = 0;
	size_t didread;
#if HAVE_MMAP
	int srcfd;
#endif

#if HAVE_MMAP
	/* try and optimize the case where we are copying from the start of a plain file.
	 * We could probably make this work in more situations, but I don't trust the stdio
	 * buffering layer.
	 * */
	if ( php_stream_is(src, PHP_STREAM_IS_STDIO) &&
			php_stream_tell(src) == 0 && 
			SUCCESS == php_stream_cast(src, PHP_STREAM_AS_FD, (void**)&srcfd, 0))
	{
		struct stat sbuf;

		if (fstat(srcfd, &sbuf) == 0) {
			void *srcfile;
			
			srcfile = mmap(NULL, sbuf.st_size, PROT_READ, MAP_SHARED, srcfd, 0);
			if (srcfile != (void*)MAP_FAILED) {
				haveread = php_stream_write(dest, srcfile, sbuf.st_size);
				munmap(srcfile, sbuf.st_size);
				return haveread;
			}
		}
		/* fall through - we might be able to copy in smaller chunks */
	}
#endif
	
	while(1) {
		readchunk = sizeof(buf);

		if (maxlen && (maxlen - haveread) < readchunk)
			readchunk = maxlen - haveread;

		didread = php_stream_read(src, buf, readchunk);
		if (didread) {
			/* extra paranoid */
			size_t didwrite, towrite;
			char *writeptr;

			towrite = didread;
			writeptr = buf;
			haveread += didread;
			
			while(towrite) {
				didwrite = php_stream_write(dest, writeptr, towrite);

				if (didwrite == 0)
					return 0;	/* error */
				
				towrite -= didwrite;
				writeptr += didwrite;
			}
		} else {
			return 0; /* error */
		}

		if (maxlen - haveread == 0) {
			break;
		}
	}

	return haveread;
	
}
/* }}} */



/* {{{ ------- STDIO stream implementation -------*/

typedef struct {
	FILE *file;
	int is_pipe;	/* use pclose */
#if HAVE_FLUSHIO
	char last_op;
#endif
} php_stdio_stream_data;

PHPAPI php_stream *php_stream_fopen_temporary_file(const char *dir, const char *pfx, char **opened_path TSRMLS_DC)
{
	FILE *fp = php_open_temporary_file(dir, pfx, opened_path);

	if (fp)	{
		php_stream *stream = php_stream_fopen_from_file(fp, "wb");
		if (stream) {
			return stream;
		}
		fclose(fp);

		zend_error(E_WARNING, "%s(): unable to allocate stream", get_active_function_name(TSRMLS_C));
		
		return NULL;
	}
	return NULL;
}

PHPAPI php_stream *php_stream_fopen_tmpfile(void)
{
	FILE *fp;
	php_stream *stream;
	
	fp = tmpfile();
	if (fp == NULL) {
		zend_error(E_WARNING, "tmpfile(): %s", strerror(errno));
		return NULL;
	}
	stream = php_stream_fopen_from_file(fp, "r+");
	if (stream == NULL)	{
		zend_error(E_WARNING, "tmpfile(): %s", strerror(errno));
		fclose(fp);
		return NULL;
	}
	return stream;
}



PHPAPI php_stream *php_stream_fopen_from_file(FILE *file, const char *mode)
{
	php_stdio_stream_data *self;

	self = emalloc(sizeof(*self));
	self->file = file;
	self->is_pipe = 0;
	return php_stream_alloc(&php_stream_stdio_ops, self, 0, mode);
}

PHPAPI php_stream *php_stream_fopen_from_pipe(FILE *file, const char *mode)
{
	php_stdio_stream_data *self;

	self = emalloc(sizeof(*self));
	self->file = file;
	self->is_pipe = 1;
	return php_stream_alloc(&php_stream_stdio_ops, self, 0, mode);
}
static size_t php_stdiop_write(php_stream *stream, const char *buf, size_t count)
{
	php_stdio_stream_data *data = (php_stdio_stream_data*)stream->abstract;

#if HAVE_FLUSHIO
	if (data->last_op == 'r') {
		fseek(data->file, 0, SEEK_CUR);
	}
	data->last_op = 'w';
#endif

	return fwrite(buf, 1, count, data->file);
}

static size_t php_stdiop_read(php_stream *stream, char *buf, size_t count)
{
	php_stdio_stream_data *data = (php_stdio_stream_data*)stream->abstract;

	if (buf == NULL && count == 0)	{
		/* check for EOF condition */
		if (feof(data->file))	{
			return EOF;
		}
		return 0;
	}

#if HAVE_FLUSHIO
	if (data->last_op == 'w')
		fseek(data->file, 0, SEEK_CUR);
	data->last_op = 'r';
#endif
	
	return fread(buf, 1, count, data->file);
}

static int php_stdiop_close(php_stream *stream)
{
	int ret;
	php_stdio_stream_data *data = (php_stdio_stream_data*)stream->abstract;

	if (data->is_pipe) {
		ret = pclose(data->file);
	} else {
		ret = fclose(data->file);
	}

	efree(data);
	
	return ret;
}

static int php_stdiop_flush(php_stream *stream)
{
	return fflush(((php_stdio_stream_data*)stream->abstract)->file);
}

static int php_stdiop_seek(php_stream *stream, off_t offset, int whence)
{
	return fseek(((php_stdio_stream_data*)stream->abstract)->file, offset, whence);
}

static char *php_stdiop_gets(php_stream *stream, char *buf, size_t size)
{
	php_stdio_stream_data *data = (php_stdio_stream_data*)stream->abstract;

#if HAVE_FLUSHIO
	if (data->last_op == 'w') {
		fseek(data->file, 0, SEEK_CUR);
	}
	data->last_op = 'r';
#endif
	
	return fgets(buf, size, data->file);
}
static int php_stdiop_cast(php_stream *stream, int castas, void **ret)
{
	int fd;
	php_stdio_stream_data *data = (php_stdio_stream_data*) stream->abstract;
 
	switch (castas)	{
		case PHP_STREAM_AS_STDIO:
			if (ret) {
				*ret = data->file;
			}
			return SUCCESS;

		case PHP_STREAM_AS_FD:
			fd = fileno(data->file);
			if (fd < 0) {
				return FAILURE;
			}
			if (ret) {
				*ret = (void*)fd;
			}
			return SUCCESS;
		default:
			return FAILURE;
	}
}

php_stream_ops	php_stream_stdio_ops = {
	php_stdiop_write, php_stdiop_read,
	php_stdiop_close, php_stdiop_flush, php_stdiop_seek,
	php_stdiop_gets, php_stdiop_cast,
	"STDIO"
};

PHPAPI php_stream *php_stream_fopen_with_path(char *filename, char *mode, char *path, char **opened_path TSRMLS_DC) /* {{{ */
{
	/* code ripped off from fopen_wrappers.c */
	char *pathbuf, *ptr, *end;
	char *exec_fname;
	char trypath[MAXPATHLEN];
	struct stat sb;
	php_stream *stream;
	int path_length;
	int filename_length;
	int exec_fname_length;

	if (opened_path) {
		*opened_path = NULL;
	}

	if(!filename) {
		return NULL;
	}

	filename_length = strlen(filename);

	/* Relative path open */
	if (*filename == '.') {
		if (PG(safe_mode) && (!php_checkuid(filename, mode, CHECKUID_CHECK_MODE_PARAM))) {
			return NULL;
		}
		return php_stream_fopen(filename, mode, opened_path TSRMLS_CC);
	}

	/*
	 * files in safe_mode_include_dir (or subdir) are excluded from
	 * safe mode GID/UID checks
	 */

	/* Absolute path open */
	if (IS_ABSOLUTE_PATH(filename, filename_length)) {
		if ((php_check_safe_mode_include_dir(filename TSRMLS_CC)) == 0)
			/* filename is in safe_mode_include_dir (or subdir) */
			return php_stream_fopen(filename, mode, opened_path TSRMLS_CC);

		if (PG(safe_mode) && (!php_checkuid(filename, mode, CHECKUID_CHECK_MODE_PARAM)))
			return NULL;

		return php_stream_fopen(filename, mode, opened_path TSRMLS_CC);
	}

	if (!path || (path && !*path)) {
		if (PG(safe_mode) && (!php_checkuid(filename, mode, CHECKUID_CHECK_MODE_PARAM))) {
			return NULL;
		}
		return php_stream_fopen(filename, mode, opened_path TSRMLS_CC);
	}

	/* check in provided path */
	/* append the calling scripts' current working directory
	 * as a fall back case
	 */
	if (zend_is_executing(TSRMLS_C)) {
		exec_fname = zend_get_executed_filename(TSRMLS_C);
		exec_fname_length = strlen(exec_fname);
		path_length = strlen(path);

		while ((--exec_fname_length >= 0) && !IS_SLASH(exec_fname[exec_fname_length]));
		if ((exec_fname && exec_fname[0] == '[')
				|| exec_fname_length<=0) {
			/* [no active file] or no path */
			pathbuf = estrdup(path);
		} else {		
			pathbuf = (char *) emalloc(exec_fname_length + path_length +1 +1);
			memcpy(pathbuf, path, path_length);
			pathbuf[path_length] = DEFAULT_DIR_SEPARATOR;
			memcpy(pathbuf+path_length+1, exec_fname, exec_fname_length);
			pathbuf[path_length + exec_fname_length +1] = '\0';
		}
	} else {
		pathbuf = estrdup(path);
	}

	ptr = pathbuf;

	while (ptr && *ptr) {
		end = strchr(ptr, DEFAULT_DIR_SEPARATOR);
		if (end != NULL) {
			*end = '\0';
			end++;
		}
		snprintf(trypath, MAXPATHLEN, "%s/%s", ptr, filename);
		if (PG(safe_mode)) {
			if (VCWD_STAT(trypath, &sb) == 0) {
				/* file exists ... check permission */
				if ((php_check_safe_mode_include_dir(trypath TSRMLS_CC) == 0) ||
						php_checkuid(trypath, mode, CHECKUID_CHECK_MODE_PARAM))
					/* UID ok, or trypath is in safe_mode_include_dir */
					stream = php_stream_fopen(trypath, mode, opened_path TSRMLS_CC);
				else
					stream = NULL;

				efree(pathbuf);
				return stream;
			}
		}
		stream = php_stream_fopen(trypath, mode, opened_path TSRMLS_CC);
		if (stream) {
			efree(pathbuf);
			return stream;
		}
		ptr = end;
	} /* end provided path */

	efree(pathbuf);
	return NULL;

}
/* }}} */

PHPAPI php_stream *php_stream_fopen(const char *filename, const char *mode, char **opened_path TSRMLS_DC)
{
	FILE *fp;
	char *realpath;

	realpath = expand_filepath(filename, NULL TSRMLS_C);
	
	fp = fopen(realpath, mode);

	if (fp)	{
		php_stream *ret = php_stream_fopen_from_file(fp, mode);

		if (ret)	{
			if (opened_path)	{
				*opened_path = realpath;
				realpath = NULL;
			}
			if (realpath)
				efree(realpath);

			return ret;
		}

		fclose(fp);
	}
	efree(realpath);
	return NULL;
}
/* }}} */

/* {{{ STDIO with fopencookie */
#if HAVE_FOPENCOOKIE
static ssize_t stream_cookie_reader(void *cookie, char *buffer, size_t size)
{
   return php_stream_read(((php_stream *)cookie), buffer, size);
}

static ssize_t stream_cookie_writer(void *cookie, const char *buffer, size_t size) {
   return php_stream_write(((php_stream *)cookie), (char *)buffer, size);
}

static int stream_cookie_seeker(void *cookie, off_t position, int whence) {
   return php_stream_seek(((php_stream *)cookie), position, whence);
}

static int stream_cookie_closer(void *cookie) {
	php_stream *stream = (php_stream*)cookie;
	/* prevent recursion */
	stream->fclose_stdiocast = PHP_STREAM_FCLOSE_NONE;
	return php_stream_close(stream);
}

static COOKIE_IO_FUNCTIONS_T stream_cookie_functions =
{
	stream_cookie_reader, stream_cookie_writer,
	stream_cookie_seeker, stream_cookie_closer
};
#else
/* TODO: use socketpair() to emulate fopencookie, as suggested by Hartmut ? */
#endif
/* }}} */

PHPAPI int php_stream_cast(php_stream *stream, int castas, void **ret, int show_err) /* {{{ */
{
	
	/* trying hard is not yet implemented */
	castas &= ~PHP_STREAM_CAST_TRY_HARD;
	
	if (castas == PHP_STREAM_AS_STDIO)	{
		if (stream->stdiocast)	{
			if (ret) {
				*ret = stream->stdiocast;
			}
			return SUCCESS;
		}

		if (stream->ops->cast && stream->ops->cast(stream, castas, ret) == SUCCESS)
			goto exit_success;


#if HAVE_FOPENCOOKIE
		/* if just checking, say yes we can be a FILE*, but don't actually create it yet */
		if (ret == NULL)
			goto exit_success;

		*ret = fopencookie(stream, stream->mode, stream_cookie_functions);

		if (*ret != NULL)	{
			stream->fclose_stdiocast = 1;
			goto exit_success;
		}

		{
			TSRMLS_FETCH();

			/* must be either:
		  		a) programmer error
		  		b) no memory
		  		-> lets bail
			 */
			zend_error(E_ERROR, "%s(): fopencookie failed", get_active_function_name(TSRMLS_C));
			return FAILURE;
		}
#endif

		goto exit_fail;
	}
	if (stream->ops->cast && stream->ops->cast(stream, castas, ret) == SUCCESS)
		goto exit_success;


exit_fail:
	if (show_err)	{
		/* these names depend on the values of the PHP_STREAM_AS_XXX defines in php_streams.h */
		static const char *cast_names[3] = {
			"STDIO FILE*", "File Descriptor", "Socket Descriptor"
		};
		TSRMLS_FETCH();

		zend_error(E_WARNING, "%s(): cannot represent a stream of type %s as a %s",
			get_active_function_name(TSRMLS_C),
			stream->ops->label,
			cast_names[castas]
			);
	}

	return FAILURE;

exit_success:
	if (castas == PHP_STREAM_AS_STDIO && ret)
		stream->stdiocast = *ret;

	return SUCCESS;

} /* }}} */

int php_init_stream_wrappers(TSRMLS_D)
{
	if (PG(allow_url_fopen))
		return zend_hash_init(&url_stream_wrappers_hash, 0, NULL, NULL, 1);
	return SUCCESS;
}

int php_shutdown_stream_wrappers(TSRMLS_D)
{
	if (PG(allow_url_fopen))
		zend_hash_destroy(&url_stream_wrappers_hash);
	return SUCCESS;
}

PHPAPI int php_register_url_stream_wrapper(char *protocol, php_stream_wrapper *wrapper TSRMLS_DC)
{
	if (PG(allow_url_fopen))
		return zend_hash_add(&url_stream_wrappers_hash, protocol, strlen(protocol), wrapper, sizeof(wrapper), NULL);
	return FAILURE;
}
PHPAPI int php_unregister_url_stream_wrapper(char *protocol TSRMLS_DC)
{
	if (PG(allow_url_fopen))
		return zend_hash_del(&url_stream_wrappers_hash, protocol, strlen(protocol));
	return SUCCESS;
}

static php_stream *php_stream_open_url(char *path, char *mode, int options, char **opened_path TSRMLS_DC)
{
	php_stream_wrapper *wrapper;
	const char *p, *protocol = NULL;
	int n = 0;

	for (p = path; isalnum((int)*p); p++) {
		n++;
	}

	if ((*p == ':') && (n > 1)) {
		protocol = path;
	}

	if (protocol)	{
		if (FAILURE == zend_hash_find(&url_stream_wrappers_hash, (char*)protocol, n, (void**)&wrapper))	{
			wrapper = NULL;
			protocol = NULL;
		}
		if (wrapper)	{
			php_stream *stream = wrapper->create(path, mode, options, opened_path TSRMLS_CC);
			if (stream)
				stream->wrapper = wrapper;
			return stream;
		}
	}

	if (!protocol || !strncasecmp(protocol, "file", n))	{
		if (protocol && path[n+1] == '/' && path[n+2] == '/')	{
			zend_error(E_WARNING, "remote host file access not supported, %s", path);
			return NULL;
		}
		if (protocol)
			path += n + 1;

		/* fall back on regular file access */
		return php_stream_open_wrapper(path, mode, (options & ~REPORT_ERRORS) | IGNORE_URL,
				opened_path TSRMLS_CC);
	}
	return NULL;
}

PHPAPI php_stream *php_stream_open_wrapper(char *path, char *mode, int options, char **opened_path TSRMLS_DC)
{
	php_stream *stream = NULL;
	
	if (opened_path)
		*opened_path = NULL;
	
	if (!path || !*path)
		return NULL;

	if (PG(allow_url_fopen) && !(options & IGNORE_URL))	{
		stream = php_stream_open_url(path, mode, options, opened_path TSRMLS_CC);
		goto out;
	}
		
	if ((options & USE_PATH) && PG(include_path) != NULL)	{
		stream = php_stream_fopen_with_path(path, mode, PG(include_path), opened_path TSRMLS_CC);
		goto out;
	}

	if ((options & ENFORCE_SAFE_MODE) && PG(safe_mode) && (!php_checkuid(path, mode, CHECKUID_CHECK_MODE_PARAM)))
		return NULL;
	
	stream = php_stream_fopen(path, mode, opened_path TSRMLS_CC);
out:
	if (stream == NULL && (options & REPORT_ERRORS))	{
		char *tmp = estrdup(path);
		php_strip_url_passwd(tmp);
		zend_error(E_WARNING, "%s(\"%s\") - %s", get_active_function_name(TSRMLS_CC), tmp, strerror(errno));
		efree(tmp);
	}
	return stream;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
