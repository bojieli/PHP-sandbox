

extern zend_function_entry php_xsl_xsltprocessor_class_functions[];
zend_class_entry *xsl_xsltprocessor_class_entry;

PHP_FUNCTION(xsl_xsltprocessor_import_stylesheet);
PHP_FUNCTION(xsl_xsltprocessor_transform_to_doc);
PHP_FUNCTION(xsl_xsltprocessor_transform_to_uri);
PHP_FUNCTION(xsl_xsltprocessor_transform_to_xml);
PHP_FUNCTION(xsl_xsltprocessor_set_parameter);
PHP_FUNCTION(xsl_xsltprocessor_get_parameter);
PHP_FUNCTION(xsl_xsltprocessor_remove_parameter);
