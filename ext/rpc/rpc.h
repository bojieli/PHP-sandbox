#ifndef RPC_H
#define RPC_H

#define FOREACH_HANDLER for (__handler_counter=0; __handler_counter < HANDLER_COUNT; __handler_counter++)
#define HANDLER handler_entries[__handler_counter]
#define HANDLER_COUNT (sizeof(handler_entries) / sizeof(rpc_handler_entry))

#define GET_INTERNAL(intern)	rpc_internal **intern;								\
								if (GET_INTERNAL_EX(intern, object) != SUCCESS) {	\
									/* TODO: exception */							\
								}

#define GET_INTERNAL_EX(intern, object)	zend_ts_hash_index_find(instance, object->value.obj.handle, (void **) &intern)

#define GET_CLASS(ce)	char *key;																				\
						int key_len;																			\
						zend_class_entry **ce;																	\
																												\
						/* the name of the rpc layer is prepended to '_load' so lets strip everything after		\
						 * the first '_' away from the function name											\
						 */																						\
						key = estrdup(get_active_function_name(TSRMLS_C));										\
						key_len = strchr(key, '_') - key;														\
						key[key_len] = '\0';																	\
																												\
						/* get the class entry for the requested rpc layer */									\
						if (zend_hash_find(CG(class_table), key, key_len + 1, (void **) &ce) != SUCCESS) {		\
							efree(key);																			\
							/* TODO: exception here */															\
						} else {																				\
							efree(key);																			\
						}

#define GET_ARGS_EX(num_args, args, args_free, strip)										\
						GET_ARGS(num_args, args)											\
																							\
						args_free = args;													\
																							\
						/* strip away the first parameters */								\
						num_args -= strip;													\
						args = (num_args > 0) ? &args[strip] : NULL;

#define GET_ARGS(num_args, args)															\
						args = (zval ***) emalloc(sizeof(zval **) * num_args);				\
																							\
						if (zend_get_parameters_array_ex(num_args, args) != SUCCESS) {		\
							efree(args);													\
							/* TODO: exception */											\
						}

#define GET_CTOR_SIGNATURE(intern, hash_val, num_args, arg_types)													\
			GET_SIGNATURE(intern, (*intern)->class_name, (*intern)->class_name_len, hash_val, num_args, arg_types)

#define GET_METHOD_SIGNATURE(intern, method, hash_val, num_args, arg_types)											\
			GET_SIGNATURE(intern, method->str, method->len, hash_val, num_args, arg_types)

#define GET_SIGNATURE(intern, name, name_len, hash_val, num_args, arg_types)										\
						hash_val.len = name_len;																	\
																													\
						if ((*(*intern)->handlers)->hash_type & HASH_WITH_SIGNATURE) {								\
							zend_uint _signature_counter;															\
																													\
							arg_types = (char *) emalloc(sizeof(char) * (num_args + 1));							\
							hash_val.len += num_args + 1;															\
																													\
							for (_signature_counter = 0; _signature_counter < num_args; _signature_counter++) {		\
								switch (Z_TYPE_PP(args[_signature_counter])) {										\
									case IS_NULL:																	\
										arg_types[_signature_counter] = 'n';										\
										break;																		\
									case IS_LONG:																	\
										arg_types[_signature_counter] = 'l';										\
										break;																		\
									case IS_DOUBLE:																	\
										arg_types[_signature_counter] = 'd';										\
										break;																		\
									case IS_STRING:																	\
										arg_types[_signature_counter] = 's';										\
										break;																		\
									case IS_ARRAY:																	\
										arg_types[_signature_counter] = 'a';										\
										break;																		\
									case IS_OBJECT:																	\
										arg_types[_signature_counter] = 'o';										\
										break;																		\
									case IS_BOOL:																	\
										arg_types[_signature_counter] = 'b';										\
										break;																		\
									case IS_RESOURCE:																\
										arg_types[_signature_counter] = 'r';										\
										break;																		\
									default:																		\
										arg_types[_signature_counter] = 'u';										\
								}																					\
							}																						\
																													\
							arg_types[_signature_counter] = '\0';													\
						} else {																					\
							arg_types = (char *) emalloc(sizeof(char));												\
							arg_types[0] = '\0';																	\
						}																							\
																													\
						hash_val.str = (char *) emalloc(sizeof(char) * (hash_val.len + 2));							\
						memcpy(hash_val.str, arg_types, num_args + 1);												\
						memcpy(&hash_val.str[hash_val.len - name_len],												\
							   name, name_len + 1);

#define FREE_SIGNATURE(hash_val, arg_types)																			\
						efree(arg_types);																			\
						efree(hash_val.str);



#define RPC_REFCOUNT(intern) ((*intern)->refcount)
#define RPC_ADDREF(intern) (++RPC_REFCOUNT(intern))
#define RPC_DELREF(intern) (--RPC_REFCOUNT(intern))

#define RPC_CLONECOUNT(intern) ((*intern)->clonecount)
#define RPC_ADDCLONE(intern) (++RPC_CLONECOUNT(intern))
#define RPC_DELCLONE(intern) (--RPC_CLONECOUNT(intern))

static int __handler_counter;

#endif