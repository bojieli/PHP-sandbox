/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2003 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Christian Stocker <chregu@php.net>                          |
   |          Rob Richards <rrichards@php.net>                            |
   |          Marcus Borger <helly@php.net>                               |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/php_rand.h"
#include "php_dom.h"
#include "dom_ce.h"
#include "dom_properties.h"

#if HAVE_DOM

#include "zend_execute_locks.h"
#include "ext/standard/info.h"
#define PHP_XPATH 1
#define PHP_XPTR 2

extern int xml_parser_inited;

zend_object_handlers dom_object_handlers;

static HashTable classes;

static HashTable dom_domstringlist_prop_handlers;
static HashTable dom_namelist_prop_handlers;
static HashTable dom_domimplementationlist_prop_handlers;
static HashTable dom_document_prop_handlers;
static HashTable dom_node_prop_handlers;
static HashTable dom_nodelist_prop_handlers;
static HashTable dom_namednodemap_prop_handlers;
static HashTable dom_characterdata_prop_handlers;
static HashTable dom_attr_prop_handlers;
static HashTable dom_element_prop_handlers;
static HashTable dom_text_prop_handlers;
static HashTable dom_typeinfo_prop_handlers;
static HashTable dom_domerror_prop_handlers;
static HashTable dom_domlocator_prop_handlers;
static HashTable dom_documenttype_prop_handlers;
static HashTable dom_notation_prop_handlers;
static HashTable dom_entity_prop_handlers;
static HashTable dom_processinginstruction_prop_handlers;


typedef int (*dom_read_t)(dom_object *obj, zval **retval TSRMLS_DC);
typedef int (*dom_write_t)(dom_object *obj, zval *newval TSRMLS_DC);

typedef struct _dom_prop_handler {
	dom_read_t read_func;
	dom_write_t write_func;
} dom_prop_handler;

static zend_function_entry dom_functions[] = {
	{NULL, NULL, NULL}
};


/* {{{ void increment_document_reference(dom_object *object) */
int increment_document_reference(dom_object *object, xmlDocPtr docp TSRMLS_DC) {
	int ret_refcount = -1;

	if (object->document != NULL) {
		object->document->refcount++;
		ret_refcount = object->document->refcount;
	} else if (docp != NULL) {
		object->document = emalloc(sizeof(dom_ref_obj));
		object->document->ptr = docp;
		object->document->refcount = 1;
		object->document->node_list = NULL;
	}

	return ret_refcount;
}
/* }}} end increment_document_reference */

/* {{{ void decrement_document_reference(dom_object *object) */
int decrement_document_reference(dom_object *object TSRMLS_DC) {
	int ret_refcount = -1;

	if (object != NULL && object->document != NULL) {
		object->document->refcount--;
		ret_refcount = object->document->refcount;
		if (ret_refcount == 0) {
			dom_clean_nodes(object TSRMLS_CC);
			node_free_resource(object->document->ptr TSRMLS_CC);
			efree(object->document);
			object->document = NULL;
		}
	}

	return ret_refcount;
}
/* }}} end decrement_document_reference */

/* {{{ dom_object_set_data */
static void dom_object_set_data(xmlNodePtr obj, dom_object *wrapper TSRMLS_DC)
{

	obj->_private = wrapper;
}
/* }}} end dom_object_set_data */

/* {{{ dom_object_get_data */
dom_object *dom_object_get_data(xmlNodePtr obj)
{
	return (dom_object *) obj->_private;
}
/* }}} end dom_object_get_data */

/* {{{ php_dom_clear_object */
static void php_dom_clear_object(dom_object *object TSRMLS_DC)
{
	object->ptr = NULL;
	if (object->prop_handler) {
		object->prop_handler = NULL;
	}
	decrement_document_reference(object TSRMLS_CC);
	object->document = NULL;
}
/* }}} end dom_object_get_data */

/* {{{ php_dom_set_object */
void php_dom_set_object(dom_object *object, void *obj TSRMLS_DC)
{
	object->ptr = obj;
	dom_object_set_data(obj, object TSRMLS_CC);
}
/* }}} end php_dom_set_object */

/* {{{ dom_unregister_node */
void dom_unregister_node(xmlNodePtr nodep TSRMLS_DC)
{
	dom_object *wrapper;

	wrapper = dom_object_get_data(nodep);
	if (wrapper != NULL ) {
		if (nodep->parent == NULL && nodep->doc != NULL && (xmlNodePtr) nodep->doc != nodep) {
			dom_del_from_list(nodep, wrapper TSRMLS_CC);
		}
		dom_object_set_data(nodep, NULL TSRMLS_CC);
		php_dom_clear_object(wrapper TSRMLS_CC);
	}
}
/* }}} end dom_unregister_node */

/* {{{ dom_del_from_list */
void dom_del_from_list(xmlNodePtr nodep, dom_object *object TSRMLS_DC)
{
	xmlNodePtr cur;
	node_list_pointer *cur_element, *prev;

	if (object != NULL && nodep != NULL) {
		if (object->document != NULL) {
			cur_element = object->document->node_list;
			prev = NULL;
			while (cur_element != NULL) {
				cur = cur_element->nodep;
				if (cur == nodep) {
					if (prev == NULL) {
						object->document->node_list = cur_element->next;
					} else {
						prev->next = cur_element->next;
					}
					efree(cur_element);
					return;
				} else {
					prev = cur_element;
					cur_element = cur_element->next;
				}
			}
		}
	}
}
/* }}} */

/* {{{ dom_add_to_list */
void dom_add_to_list(xmlNodePtr nodep, dom_object *object TSRMLS_DC)
{
	node_list_pointer *cur_element;

	if (object != NULL && nodep != NULL) {
		if (nodep->parent == NULL) {
			if (object->document != NULL) {
				dom_del_from_list(nodep, object TSRMLS_CC);
				cur_element = emalloc(sizeof(node_list_pointer));
				cur_element->nodep = nodep;
				cur_element->next = NULL;
				if (object->document->node_list != NULL) {
					cur_element->next = object->document->node_list;
				}
				object->document->node_list = cur_element;
			}
		}
	}
}
/* }}} */

/* {{{ dom_read_na */
static int dom_read_na(dom_object *obj, zval **retval TSRMLS_DC)
{
	*retval = NULL;
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot read property");
	return FAILURE;
}
/* }}} */

/* {{{ dom_write_na */
static int dom_write_na(dom_object *obj, zval *newval TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot write property");
	return FAILURE;
}
/* }}} */

/* {{{ dom_register_prop_handler */
static void dom_register_prop_handler(HashTable *prop_handler, char *name, dom_read_t read_func, dom_write_t write_func TSRMLS_DC)
{
	dom_prop_handler hnd;
	
	hnd.read_func = read_func ? read_func : dom_read_na;
	hnd.write_func = write_func ? write_func : dom_write_na;
	zend_hash_add(prop_handler, name, strlen(name)+1, &hnd, sizeof(dom_prop_handler), NULL);
}
/* }}} */

/* {{{ dom_read_property */
zval *dom_read_property(zval *object, zval *member TSRMLS_DC)
{
	dom_object *obj;
	zval tmp_member;
	zval *retval;
	dom_prop_handler *hnd;
	zend_object_handlers *std_hnd;
	int ret;

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	ret = FAILURE;
	obj = (dom_object *)zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}
	if (ret == SUCCESS) {
		ret = hnd->read_func(obj, &retval TSRMLS_CC);
		if (ret == SUCCESS) {
			/* ensure we're creating a temporary variable */
			retval->refcount = 1;
			PZVAL_UNLOCK(retval);
		} else {
			retval = EG(uninitialized_zval_ptr);
		}
	} else {
		std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->read_property(object, member TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
	return retval;
}
/* }}} */

/* {{{ dom_write_property */
void dom_write_property(zval *object, zval *member, zval *value TSRMLS_DC)
{
	dom_object *obj;
	zval tmp_member;
	dom_prop_handler *hnd;
	zend_object_handlers *std_hnd;
	int ret;

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	ret = FAILURE;
	obj = (dom_object *)zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find((HashTable *)obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}
	if (ret == SUCCESS) {
		hnd->write_func(obj, value TSRMLS_CC);
	} else {
		std_hnd = zend_get_std_object_handlers();
		std_hnd->write_property(object, member, value TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
}
/* }}} */

zend_module_entry dom_module_entry = {
	STANDARD_MODULE_HEADER,
	"dom",
	dom_functions,
	PHP_MINIT(dom),
	PHP_MSHUTDOWN(dom),
	NULL,
	NULL,
	PHP_MINFO(dom),
	DOM_API_VERSION, /* Extension versionnumber */
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_DOM
ZEND_GET_MODULE(dom)
#endif

/* {{{ PHP_MINIT_FUNCTION(dom) */
PHP_MINIT_FUNCTION(dom)
{
	zend_class_entry ce;

	memcpy(&dom_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dom_object_handlers.read_property = dom_read_property;
	dom_object_handlers.write_property = dom_write_property;

	zend_hash_init(&classes, 0, NULL, NULL, 1);

	REGISTER_DOM_CLASS(ce, "domexception", NULL, php_dom_domexception_class_functions, dom_domexception_class_entry);
	REGISTER_DOM_CLASS(ce, "domstringlist", NULL, php_dom_domstringlist_class_functions, dom_domstringlist_class_entry);
	
	zend_hash_init(&dom_domstringlist_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_domstringlist_prop_handlers, "length", dom_domstringlist_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_domstringlist_prop_handlers, sizeof(dom_domstringlist_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domnamelist", NULL, php_dom_namelist_class_functions, dom_namelist_class_entry);
	
	zend_hash_init(&dom_namelist_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_namelist_prop_handlers, "length", dom_namelist_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_namelist_prop_handlers, sizeof(dom_namelist_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domimplementationlist", NULL, php_dom_domimplementationlist_class_functions, dom_domimplementationlist_class_entry);
	
	zend_hash_init(&dom_domimplementationlist_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_domimplementationlist_prop_handlers, "length", dom_domimplementationlist_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_domimplementationlist_prop_handlers, sizeof(dom_domimplementationlist_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domimplementationsource", NULL, php_dom_domimplementationsource_class_functions, dom_domimplementationsource_class_entry);
	REGISTER_DOM_CLASS(ce, "domimplementation", NULL, php_dom_domimplementation_class_functions, dom_domimplementation_class_entry);

	REGISTER_DOM_CLASS(ce, "domnode", NULL, php_dom_node_class_functions, dom_node_class_entry);
	
	zend_hash_init(&dom_node_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_node_prop_handlers, "nodeName", dom_node_node_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "nodeValue", dom_node_node_value_read, dom_node_node_value_write TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "nodeType", dom_node_node_type_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "parentNode", dom_node_parent_node_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "childNodes", dom_node_child_nodes_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "firstChild", dom_node_first_child_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "lastChild", dom_node_last_child_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "previousSibling", dom_node_previous_sibling_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "nextSibling", dom_node_next_sibling_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "attributes", dom_node_attributes_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "ownerDocument", dom_node_owner_document_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "namespaceURI", dom_node_namespace_uri_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "prefix", dom_node_prefix_read, dom_node_prefix_write TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "localName", dom_node_local_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "baseURI", dom_node_base_uri_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "textContent", dom_node_text_content_read, dom_node_text_content_write TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_node_prop_handlers, sizeof(dom_node_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domdocumentfragment", dom_node_class_entry, php_dom_documentfragment_class_functions, dom_documentfragment_class_entry);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_node_prop_handlers, sizeof(dom_node_prop_handlers), NULL);
	
	REGISTER_DOM_CLASS(ce, "domdocument", dom_node_class_entry, php_dom_document_class_functions, dom_document_class_entry);
	zend_hash_init(&dom_document_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_document_prop_handlers, "doctype", dom_document_doctype_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "implementation", dom_document_implementation_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "documentElement", dom_document_document_element_read, NULL TSRMLS_CC);
/* actualEncoding currently set as read only alias to encoding
	dom_register_prop_handler(&dom_document_prop_handlers, "actualEncoding", dom_document_actual_encoding_read, dom_document_actual_encoding_write TSRMLS_CC); */
	dom_register_prop_handler(&dom_document_prop_handlers, "actualEncoding", dom_document_encoding_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "encoding", dom_document_encoding_read, dom_document_encoding_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "standalone", dom_document_standalone_read, dom_document_standalone_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "version", dom_document_version_read, dom_document_version_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "strictErrorChecking", dom_document_strict_error_checking_read, dom_document_strict_error_checking_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "documentURI", dom_document_document_uri_read, dom_document_document_uri_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "config", dom_document_config_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_document_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_document_prop_handlers, sizeof(dom_document_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domnodelist", NULL, php_dom_nodelist_class_functions, dom_nodelist_class_entry);
	
	zend_hash_init(&dom_nodelist_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_nodelist_prop_handlers, "length", dom_nodelist_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_nodelist_prop_handlers, sizeof(dom_nodelist_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domnamednodemap", NULL, php_dom_namednodemap_class_functions, dom_namednodemap_class_entry);
	
	zend_hash_init(&dom_namednodemap_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_namednodemap_prop_handlers, "length", dom_namednodemap_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_namednodemap_prop_handlers, sizeof(dom_namednodemap_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domcharacterdata", dom_node_class_entry, php_dom_characterdata_class_functions, dom_characterdata_class_entry);
	
	zend_hash_init(&dom_characterdata_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_characterdata_prop_handlers, "data", dom_characterdata_data_read, dom_characterdata_data_write TSRMLS_CC);
	dom_register_prop_handler(&dom_characterdata_prop_handlers, "length", dom_characterdata_length_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_characterdata_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_characterdata_prop_handlers, sizeof(dom_characterdata_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domattr", dom_node_class_entry, php_dom_attr_class_functions, dom_attr_class_entry);
	
	zend_hash_init(&dom_attr_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_attr_prop_handlers, "name", dom_attr_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_attr_prop_handlers, "specified", dom_attr_specified_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_attr_prop_handlers, "value", dom_attr_value_read, dom_attr_value_write TSRMLS_CC);
	dom_register_prop_handler(&dom_attr_prop_handlers, "ownerElement", dom_attr_owner_element_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_attr_prop_handlers, "schemaTypeInfo", dom_attr_schema_type_info_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_attr_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_attr_prop_handlers, sizeof(dom_attr_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domelement", dom_node_class_entry, php_dom_element_class_functions, dom_element_class_entry);
	
	zend_hash_init(&dom_element_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_element_prop_handlers, "tagName", dom_element_tag_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_element_prop_handlers, "schemaTypeInfo", dom_element_schema_type_info_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_element_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_element_prop_handlers, sizeof(dom_element_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domtext", dom_characterdata_class_entry, php_dom_text_class_functions, dom_text_class_entry);
	
	zend_hash_init(&dom_text_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_text_prop_handlers, "wholeText", dom_text_whole_text_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_text_prop_handlers, &dom_characterdata_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_text_prop_handlers, sizeof(dom_text_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domcomment", dom_characterdata_class_entry, php_dom_comment_class_functions, dom_comment_class_entry);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_characterdata_prop_handlers, sizeof(dom_typeinfo_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domtypeinfo", NULL, php_dom_typeinfo_class_functions, dom_typeinfo_class_entry);
	
	zend_hash_init(&dom_typeinfo_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_typeinfo_prop_handlers, "typeName", dom_typeinfo_type_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_typeinfo_prop_handlers, "typeNamespace", dom_typeinfo_type_namespace_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_typeinfo_prop_handlers, sizeof(dom_typeinfo_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domuserdatahandler", NULL, php_dom_userdatahandler_class_functions, dom_userdatahandler_class_entry);
	REGISTER_DOM_CLASS(ce, "domdomerror", NULL, php_dom_domerror_class_functions, dom_domerror_class_entry);
	
	zend_hash_init(&dom_domerror_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "severity", dom_domerror_severity_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "message", dom_domerror_message_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "type", dom_domerror_type_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "relatedException", dom_domerror_related_exception_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "related_data", dom_domerror_related_data_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "location", dom_domerror_location_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_domerror_prop_handlers, sizeof(dom_domerror_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domerrorhandler", NULL, php_dom_domerrorhandler_class_functions, dom_domerrorhandler_class_entry);
	REGISTER_DOM_CLASS(ce, "domlocator", NULL, php_dom_domlocator_class_functions, dom_domlocator_class_entry);
	
	zend_hash_init(&dom_domlocator_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "lineNumber", dom_domlocator_line_number_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "columnNumber", dom_domlocator_column_number_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "offset", dom_domlocator_offset_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "relatedNode", dom_domlocator_related_node_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "uri", dom_domlocator_uri_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_domlocator_prop_handlers, sizeof(dom_domlocator_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domconfiguration", NULL, php_dom_domconfiguration_class_functions, dom_domconfiguration_class_entry);
	REGISTER_DOM_CLASS(ce, "domcdatasection", dom_text_class_entry, php_dom_cdatasection_class_functions, dom_cdatasection_class_entry);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_text_prop_handlers, sizeof(dom_documenttype_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domdocumenttype", dom_node_class_entry, php_dom_documenttype_class_functions, dom_documenttype_class_entry);
	
	zend_hash_init(&dom_documenttype_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "name", dom_documenttype_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "entities", dom_documenttype_entities_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "notations", dom_documenttype_notations_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "publicId", dom_documenttype_public_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "systemId", dom_documenttype_system_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "internalSubset", dom_documenttype_internal_subset_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_documenttype_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_documenttype_prop_handlers, sizeof(dom_documenttype_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domnotation", dom_node_class_entry, php_dom_notation_class_functions, dom_notation_class_entry);
	
	zend_hash_init(&dom_notation_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_notation_prop_handlers, "publicId", dom_notation_public_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_notation_prop_handlers, "systemId", dom_notation_system_id_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_notation_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_notation_prop_handlers, sizeof(dom_notation_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domentity", dom_node_class_entry, php_dom_entity_class_functions, dom_entity_class_entry);
	
	zend_hash_init(&dom_entity_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_entity_prop_handlers, "publicId", dom_entity_public_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "systemId", dom_entity_system_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "notationName", dom_entity_notation_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "actualEncoding", dom_entity_actual_encoding_read, dom_entity_actual_encoding_write TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "encoding", dom_entity_encoding_read, dom_entity_encoding_write TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "version", dom_entity_version_read, dom_entity_version_write TSRMLS_CC);
	zend_hash_merge(&dom_entity_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);

	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_entity_prop_handlers, sizeof(dom_entity_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domentityreference", dom_node_class_entry, php_dom_entityreference_class_functions, dom_entityreference_class_entry);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_node_prop_handlers, sizeof(dom_entity_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domprocessinginstruction", dom_node_class_entry, php_dom_processinginstruction_class_functions, dom_processinginstruction_class_entry);
	
	zend_hash_init(&dom_processinginstruction_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_processinginstruction_prop_handlers, "target", dom_processinginstruction_target_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_processinginstruction_prop_handlers, "data", dom_processinginstruction_data_read, dom_processinginstruction_data_write TSRMLS_CC);
	zend_hash_merge(&dom_processinginstruction_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_processinginstruction_prop_handlers, sizeof(dom_processinginstruction_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domstring_extend", NULL, php_dom_string_extend_class_functions, dom_string_extend_class_entry);

	REGISTER_LONG_CONSTANT("XML_ELEMENT_NODE",			XML_ELEMENT_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NODE",		XML_ATTRIBUTE_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_TEXT_NODE",				XML_TEXT_NODE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_CDATA_SECTION_NODE",	XML_CDATA_SECTION_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_REF_NODE",		XML_ENTITY_REF_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_NODE",			XML_ENTITY_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_PI_NODE",				XML_PI_NODE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_COMMENT_NODE",			XML_COMMENT_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_NODE",			XML_DOCUMENT_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_TYPE_NODE",	XML_DOCUMENT_TYPE_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_FRAG_NODE",	XML_DOCUMENT_FRAG_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_NOTATION_NODE",			XML_NOTATION_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_HTML_DOCUMENT_NODE",	XML_HTML_DOCUMENT_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DTD_NODE",				XML_DTD_NODE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ELEMENT_DECL_NODE", 	XML_ELEMENT_DECL,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_DECL_NODE",	XML_ATTRIBUTE_DECL,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_DECL_NODE",		XML_ENTITY_DECL,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_NAMESPACE_DECL_NODE",	XML_NAMESPACE_DECL,			CONST_CS | CONST_PERSISTENT);
#ifdef XML_GLOBAL_NAMESPACE
	REGISTER_LONG_CONSTANT("XML_GLOBAL_NAMESPACE",		XML_GLOBAL_NAMESPACE,		CONST_CS | CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("XML_LOCAL_NAMESPACE",		XML_LOCAL_NAMESPACE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_CDATA",		XML_ATTRIBUTE_CDATA,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ID",			XML_ATTRIBUTE_ID,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_IDREF",		XML_ATTRIBUTE_IDREF,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_IDREFS",		XML_ATTRIBUTE_IDREFS,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ENTITY",		XML_ATTRIBUTE_ENTITIES,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NMTOKEN",		XML_ATTRIBUTE_NMTOKEN,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NMTOKENS",	XML_ATTRIBUTE_NMTOKENS,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ENUMERATION",	XML_ATTRIBUTE_ENUMERATION,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NOTATION",	XML_ATTRIBUTE_NOTATION,		CONST_CS | CONST_PERSISTENT);

	if (!xml_parser_inited) {
		xmlInitThreads();
		xml_parser_inited = 1;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ */
PHP_MINFO_FUNCTION(dom)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "DOM/XML", "enabled");
	php_info_print_table_row(2, "DOM/XML API Version", DOM_API_VERSION);
	php_info_print_table_row(2, "libxml Version", xmlParserVersion);
#if defined(LIBXML_HTML_ENABLED)
	php_info_print_table_row(2, "HTML Support", "enabled");
#endif
#if defined(LIBXML_XPATH_ENABLED)
	php_info_print_table_row(2, "XPath Support", "enabled");
#endif
#if defined(LIBXML_XPTR_ENABLED)
	php_info_print_table_row(2, "XPointer Support", "enabled");
#endif
	php_info_print_table_end();
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(dom)
{
   	if (xml_parser_inited) {
		xmlCleanupParser();
		xml_parser_inited = 0;
	}

	zend_hash_destroy(&dom_domstringlist_prop_handlers);
	zend_hash_destroy(&dom_namelist_prop_handlers);
	zend_hash_destroy(&dom_domimplementationlist_prop_handlers);
	zend_hash_destroy(&dom_document_prop_handlers);
	zend_hash_destroy(&dom_node_prop_handlers);
	zend_hash_destroy(&dom_nodelist_prop_handlers);
	zend_hash_destroy(&dom_namednodemap_prop_handlers);
	zend_hash_destroy(&dom_characterdata_prop_handlers);
	zend_hash_destroy(&dom_attr_prop_handlers);
	zend_hash_destroy(&dom_element_prop_handlers);
	zend_hash_destroy(&dom_text_prop_handlers);
	zend_hash_destroy(&dom_typeinfo_prop_handlers);
	zend_hash_destroy(&dom_domerror_prop_handlers);
	zend_hash_destroy(&dom_domlocator_prop_handlers);
	zend_hash_destroy(&dom_documenttype_prop_handlers);
	zend_hash_destroy(&dom_notation_prop_handlers);
	zend_hash_destroy(&dom_entity_prop_handlers);
	zend_hash_destroy(&dom_processinginstruction_prop_handlers);
	zend_hash_destroy(&classes);
	
/*	If you want do find memleaks in this module, compile libxml2 with --with-mem-debug and
	uncomment the following line, this will tell you the amount of not freed memory
	and the total used memory into apaches error_log  */
/*  xmlMemoryDump();*/

	return SUCCESS;
}

/* {{{ dom_clean_nodes */
void dom_clean_nodes(dom_object *object TSRMLS_DC)
{
	node_list_pointer *l;

	if (object != NULL && object->document != NULL) {
		l = object->document->node_list;
		while (l != NULL) {
			node_free_resource(l->nodep TSRMLS_CC);
			l = object->document->node_list;
		}
	}
}
/* }}} */

/* {{{ node_list_unlink */
void node_list_unlink(xmlNodePtr node TSRMLS_DC)
{
	dom_object *wrapper;

	while (node != NULL) {

		wrapper = dom_object_get_data(node);

		if (wrapper != NULL ) {
			dom_add_to_list(node, wrapper TSRMLS_CC);
			xmlUnlinkNode(node);
		} else {
			node_list_unlink(node->children TSRMLS_CC);

			switch (node->type) {
				case XML_ATTRIBUTE_DECL:
				case XML_DTD_NODE:
				case XML_DOCUMENT_TYPE_NODE:
				case XML_ENTITY_DECL:
				case XML_ATTRIBUTE_NODE:
					break;
				default:
					node_list_unlink((xmlNodePtr) node->properties TSRMLS_CC);
			}

		}

		node = node->next;
	}
}
/* }}} end node_list_unlink */


/* {{{ void dom_node_free(xmlNodePtr node) */
void dom_node_free(xmlNodePtr node)
{
	if(node) {
		switch (node->type) {
			case XML_ATTRIBUTE_NODE:
				xmlFreeProp((xmlAttrPtr) node);
				break;
			case XML_ENTITY_DECL:
				break;
			default:
				xmlFreeNode(node);
		}
	}
}
/* }}} end dom_node_free */

/* {{{ node_free_list */
void node_free_list(xmlNodePtr node TSRMLS_DC)
{
	xmlNodePtr curnode;

	if (node != NULL) {
		curnode = node;
		while (curnode != NULL) {
			node = curnode;
			switch (node->type) {
				/* Skip property freeing for the following types */
				case XML_ENTITY_REF_NODE:
					node_free_list((xmlNodePtr) node->properties TSRMLS_CC);
					break;
				case XML_ATTRIBUTE_DECL:
				case XML_DTD_NODE:
				case XML_DOCUMENT_TYPE_NODE:
				case XML_ENTITY_DECL:
				case XML_ATTRIBUTE_NODE:
					node_free_list(node->children TSRMLS_CC);
					break;
				default:
					node_free_list(node->children TSRMLS_CC);
					node_free_list((xmlNodePtr) node->properties TSRMLS_CC);
			}
			
			curnode = node->next;
			xmlUnlinkNode(node);
			dom_unregister_node(node TSRMLS_CC);
			dom_node_free(node);
		}
	}
}
/* }}} end node_free_list */

/* {{{ node_free_resource */
void node_free_resource(xmlNodePtr node TSRMLS_DC)
{
	xmlDtdPtr extSubset, intSubset;
	xmlDocPtr docp;

	if (!node) {
		return;
	}

	switch (node->type) {
		case XML_DOCUMENT_NODE:
		case XML_HTML_DOCUMENT_NODE:
		{
			docp = (xmlDocPtr) node;
			if (docp->ids != NULL) xmlFreeIDTable((xmlIDTablePtr) docp->ids);
			docp->ids = NULL;
			if (docp->refs != NULL) xmlFreeRefTable((xmlRefTablePtr) docp->refs);
			docp->refs = NULL;
			extSubset = docp->extSubset;
			intSubset = docp->intSubset;
			if (intSubset == extSubset)
				extSubset = NULL;
			if (extSubset != NULL) {
				node_free_list((xmlNodePtr) extSubset->children TSRMLS_CC);
				xmlUnlinkNode((xmlNodePtr) docp->extSubset);
				docp->extSubset = NULL;
				xmlFreeDtd(extSubset);
			}
			if (intSubset != NULL) {
				node_free_list((xmlNodePtr) intSubset->children TSRMLS_CC);
				xmlUnlinkNode((xmlNodePtr) docp->intSubset);
				docp->intSubset = NULL;
				xmlFreeDtd(intSubset);
			}

			node_free_list(node->children TSRMLS_CC);
			node_free_list((xmlNodePtr) node->properties TSRMLS_CC);
			xmlFreeDoc((xmlDoc *) node);
			break;
		}
		default:
			if (node->parent == NULL) {
				node_free_list((xmlNodePtr) node->children TSRMLS_CC);
				switch (node->type) {
					/* Skip property freeing for the following types */
					case XML_ATTRIBUTE_DECL:
					case XML_DTD_NODE:
					case XML_DOCUMENT_TYPE_NODE:
					case XML_ENTITY_DECL:
					case XML_ATTRIBUTE_NODE:
						break;
					default:
						node_free_list((xmlNodePtr) node->properties TSRMLS_CC);
				}
				dom_unregister_node(node TSRMLS_CC);
				switch (node->type) {
					case XML_ATTRIBUTE_NODE:
						xmlFreeProp((xmlAttrPtr) node);
						break;
					default:
						xmlFreeNode((xmlNode *) node);
				}
			} else {
				dom_unregister_node(node TSRMLS_CC);
			}
	}
}
/* }}} */

/* {{{ dom_objects_clone */
void dom_objects_clone(void *object, void **object_clone TSRMLS_DC)
{
	/* TODO */
}
/* }}} */

/* {{{ dom_objects_dtor */
void dom_objects_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	dom_object *intern = (dom_object *)object;
	int retcount;

	zend_hash_destroy(intern->std.properties);
	FREE_HASHTABLE(intern->std.properties);



	if (intern->ptr) {
		if (((xmlNodePtr) intern->ptr)->type != XML_DOCUMENT_NODE && ((xmlNodePtr) intern->ptr)->type != XML_HTML_DOCUMENT_NODE) {
			node_free_resource(intern->ptr TSRMLS_CC);
		} else {
			retcount = decrement_document_reference(intern TSRMLS_CC);
			if (retcount != 0) {
				dom_object_set_data(intern->ptr, NULL TSRMLS_CC);
			}
			intern->document = NULL;
		}
		intern->ptr = NULL;
	}

	efree(object);
}
/* }}} */

/* {{{ dom_objects_new */
zend_object_value dom_objects_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	dom_object *intern;
	zend_class_entry *base_class;
	zval *tmp;

	intern = emalloc(sizeof(dom_object));
	intern->std.ce = class_type;
	intern->std.in_get = 0;
	intern->std.in_set = 0;
	intern->ptr = NULL;
	intern->prop_handler = NULL;
	intern->document = NULL;
	
	base_class = class_type;
	while(base_class->type != ZEND_INTERNAL_CLASS && base_class->parent != NULL) {
		base_class = base_class->parent;
	}

	zend_hash_find(&classes, base_class->name , base_class->name_length + 1, (void **) &intern->prop_handler);

	ALLOC_HASHTABLE(intern->std.properties);
	zend_hash_init(intern->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(intern, dom_objects_dtor, dom_objects_clone TSRMLS_CC);
	intern->handle = retval.handle;
	retval.handlers = &dom_object_handlers;

	return retval;
}
/* }}} */

/* {{{ php_domobject_new */
zval *php_dom_create_object(xmlNodePtr obj, int *found, zval *wrapper_in, zval *return_value, dom_object *domobj TSRMLS_DC)
{
	zval *wrapper;
	zend_class_entry *ce;
	dom_object *intern;

	*found = 0;

	if (!obj) {
		ALLOC_ZVAL(wrapper);
		ZVAL_NULL(wrapper);
		return wrapper;
	}

	if ((intern = (dom_object *) dom_object_get_data((void *) obj))) {
		return_value->type = IS_OBJECT;
		return_value->is_ref = 1;
		return_value->value.obj.handle = intern->handle;
		return_value->value.obj.handlers = &dom_object_handlers;
		zval_copy_ctor(return_value);
		*found = 1;
		return return_value;
	}

	wrapper = return_value;

	switch (obj->type) {
		case XML_DOCUMENT_NODE:
		case XML_HTML_DOCUMENT_NODE:
		{
			ce = dom_document_class_entry;
			break;
		}
		case XML_DTD_NODE:
		case XML_DOCUMENT_TYPE_NODE:
		{
			ce = dom_documenttype_class_entry;
			break;
		}
		case XML_ELEMENT_NODE:
		{
			ce = dom_element_class_entry;
			break;
		}
		case XML_ATTRIBUTE_NODE:
		{
			ce = dom_attr_class_entry;
			break;
		}
		case XML_TEXT_NODE:
		{
			ce = dom_text_class_entry;
			break;
		}
		case XML_COMMENT_NODE:
		{
			ce = dom_comment_class_entry;
			break;
		}
		case XML_PI_NODE:
		{
			ce = dom_processinginstruction_class_entry;
			break;
		}
		case XML_ENTITY_REF_NODE:
		{
			ce = dom_entityreference_class_entry;
			break;
		}
		case XML_ENTITY_DECL:
		case XML_ELEMENT_DECL:
		{
			ce = dom_entity_class_entry;
			break;
		}
		case XML_CDATA_SECTION_NODE:
		{
			ce = dom_cdatasection_class_entry;
			break;
		}
		case XML_DOCUMENT_FRAG_NODE:
		{
			ce = dom_documentfragment_class_entry;
			break;
		}
		case XML_NOTATION_NODE:
		{
			ce = dom_notation_class_entry;
			break;
		}
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported node type: %d\n", Z_TYPE_P(obj));
			ZVAL_NULL(wrapper);
			return wrapper;
	}


	object_init_ex(wrapper, ce);
	intern = (dom_object *)zend_objects_get_address(wrapper TSRMLS_CC);
	if (obj->doc != NULL) {
		if (domobj != NULL) {
			intern->document = domobj->document;
		}
		increment_document_reference(intern, obj->doc TSRMLS_CC);
	}

	php_dom_set_object(intern, (void *) obj TSRMLS_CC);
	return (wrapper);
}
/* }}} end php_domobject_new */


void php_dom_create_implementation(zval **retval  TSRMLS_DC) {
	object_init_ex(*retval, dom_domimplementation_class_entry);
}

/* {{{ int dom_hierarchy(xmlNodePtr parent, xmlNodePtr child) */
int dom_hierarchy(xmlNodePtr parent, xmlNodePtr child) 
{
	xmlNodePtr nodep;

    if (parent == NULL || child == NULL || child->doc != parent->doc) {
        return SUCCESS;
    }

	nodep = parent;

	while (nodep) {
		if (nodep == child) {
			return FAILURE;
		}
		nodep = nodep->parent;
	}

    return SUCCESS;
}
/* }}} end dom_hierarchy */

/* {{{ void dom_element_get_elements_by_tag_name_ns_raw(xmlNodePtr nodep, char *ns, char *local, zval **retval  TSRMLS_DC) */
void dom_get_elements_by_tag_name_ns_raw(xmlNodePtr nodep, char *ns, char *local, zval **retval, dom_object *intern  TSRMLS_DC)
{
	dom_object *wrapper;
	int ret;

	while (nodep != NULL) {
		if (nodep->type == XML_ELEMENT_NODE && xmlStrEqual(nodep->name, local)) {
			if (ns == NULL || (nodep->ns != NULL && xmlStrEqual(nodep->ns->href, ns))) {
				zval *child = NULL;
				wrapper = dom_object_get_data(nodep);
				if (wrapper == NULL) {
					MAKE_STD_ZVAL(child);
				}

				child = php_dom_create_object(nodep, &ret, NULL, child, intern TSRMLS_CC);
				add_next_index_zval(*retval, child);
			}
		}
		dom_get_elements_by_tag_name_ns_raw(nodep->children, ns, local, retval, intern TSRMLS_CC);
		nodep = nodep->next;
	}
}
/* }}} end dom_element_get_elements_by_tag_name_ns_raw */


/* {{{ void dom_normalize (xmlNodePtr nodep TSRMLS_DC) */
void dom_normalize (xmlNodePtr nodep TSRMLS_DC)
{
	xmlNodePtr child, nextp;
	xmlAttrPtr attr;
	xmlChar	*strContent; 

	child = nodep->children;
	while(child != NULL) {
		switch (child->type) {
			case XML_TEXT_NODE:
				nextp = child->next;
				while (nextp != NULL) {
					if (nextp->type == XML_TEXT_NODE) {
						strContent = xmlNodeGetContent(nextp);
						xmlNodeAddContent(child, strContent);
						xmlFree(strContent);
						xmlUnlinkNode(nextp);
						node_free_resource(nextp TSRMLS_CC);
						nextp = child->next;
					}
				}
				break;
			case XML_ELEMENT_NODE:
				dom_normalize (child TSRMLS_CC);
				attr = child->properties;
				while (attr != NULL) {
					dom_normalize((xmlNodePtr) attr TSRMLS_CC);
					attr = attr->next;
				}
				break;
			case XML_ATTRIBUTE_NODE:
				dom_normalize (child TSRMLS_CC);
				break;
			default:
				break;
		}
		child = child->next;
	}
}
/* }}} end dom_normalize */


/* {{{ void dom_set_old_ns(xmlDoc *doc, xmlNs *ns) */
void dom_set_old_ns(xmlDoc *doc, xmlNs *ns) {
	xmlNs *cur;

	if (doc == NULL)
		return;

	if (doc->oldNs == NULL) {
		doc->oldNs = (xmlNsPtr) xmlMalloc(sizeof(xmlNs));
		if (doc->oldNs == NULL) {
			return;
		}
		memset(doc->oldNs, 0, sizeof(xmlNs));
		doc->oldNs->type = XML_LOCAL_NAMESPACE;
		doc->oldNs->href = xmlStrdup(XML_XML_NAMESPACE); 
		doc->oldNs->prefix = xmlStrdup((const xmlChar *)"xml"); 
	}

	cur = doc->oldNs;
	while (cur->next != NULL) {
		cur = cur->next;
	}
	cur->next = ns;
}
/* }}} end dom_set_old_ns */


/* {{{ xmlNsPtr dom_get_ns(char *uri, char *qName, int uri_len, int qName_len, int *errorcode, char *localname) */
xmlNsPtr dom_get_ns(char *uri, char *qName, int uri_len, int qName_len, int *errorcode, char **localname) {
	xmlNsPtr nsptr = NULL;
	xmlURIPtr uristruct;
	char *prefix = NULL;

	*localname = NULL;
	*errorcode = 0;

	if (uri_len > 0 || qName_len > 0) {
		if (qName_len == 0 && uri_len > 0) {
			*errorcode = NAMESPACE_ERR;
			return nsptr;
		}
		if (qName_len > 0 && *errorcode == 0) {
			uristruct = xmlParseURI(qName);
			if (uristruct->opaque != NULL) {
				prefix = xmlStrdup(uristruct->scheme);
				*localname = xmlStrdup(uristruct->opaque);
				if (xmlStrchr(*localname, (xmlChar) ':') != NULL) {
					*errorcode = NAMESPACE_ERR;
				} else if (!strcmp (prefix, "xml") && strcmp(uri, XML_XML_NAMESPACE)) {
					*errorcode = NAMESPACE_ERR;
				}
			}

			/* TODO: Test that localname has no invalid chars 
			php_dom_throw_error(INVALID_CHARACTER_ERR, TSRMLS_CC);
			*/

			xmlFreeURI(uristruct);
			
			if (*errorcode == 0) {
				if (uri_len > 0 && prefix == NULL) {
					*errorcode = NAMESPACE_ERR;
				} else if (*localname != NULL) {
					nsptr = xmlNewNs(NULL, uri, prefix);
				}
			}
		}
	}

	if (prefix != NULL) {
		xmlFree(prefix);
	}

	return nsptr;

}
/* }}} end dom_get_ns */

/* {{{ xmlNsPtr dom_get_nsdecl(xmlNode *node, xmlChar *localName) */
xmlNsPtr dom_get_nsdecl(xmlNode *node, xmlChar *localName) {
	xmlNsPtr cur;
	xmlNs *ret = NULL;
	if (node == NULL)
		return NULL;

	if (localName == NULL || xmlStrEqual(localName, "")) {
		cur = node->nsDef;
		while (cur != NULL) {
			if (cur->prefix == NULL) {
				ret = cur;
				break;
			}
			cur = cur->next;
		}
	} else {
		cur = node->nsDef;
		while (cur != NULL) {
			if (cur->prefix != NULL && xmlStrEqual(localName, cur->prefix)) {
				ret = cur;
				break;
			}
			cur = cur->next;
		}
	}
	return ret;
}
/* }}} end dom_get_nsdecl */

#endif /* HAVE_DOM */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
