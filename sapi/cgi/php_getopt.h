/* Borrowed from Apache NT Port */
#include "php.h"

extern char *ap_php_optarg;
extern int ap_php_optind;
extern int ap_php_opterr;
extern int ap_php_optopt;

int ap_php_getopt(int argc, char* const *argv, const char *optstr);
