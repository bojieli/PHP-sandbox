<?php
//
// +----------------------------------------------------------------------+
// | PHP version 4.0                                                      |
// +----------------------------------------------------------------------+
// | Copyright (c) 1997-2001 The PHP Group                                |
// +----------------------------------------------------------------------+
// | This source file is subject to version 2.0 of the PHP license,       |
// | that is bundled with this package in the file LICENSE, and is        |
// | available at through the world-wide-web at                           |
// | http://www.php.net/license/2_02.txt.                                 |
// | If you did not receive a copy of the PHP license and are unable to   |
// | obtain it through the world-wide-web, please send a note to          |
// | license@php.net so we can mail you a copy immediately.               |
// +----------------------------------------------------------------------+
// | Authors: Sterling Hughes <sterling@php.net>                          |
// |          Stig Bakken <ssb@fast.no>                                   |
// +----------------------------------------------------------------------+
//
// $Id$
//

define('PEAR_ERROR_RETURN', 1);
define('PEAR_ERROR_PRINT', 2);
define('PEAR_ERROR_TRIGGER', 4);
define('PEAR_ERROR_DIE', 8);
define('PEAR_ERROR_CALLBACK', 16);

if (substr(PHP_OS, 0, 3) == 'WIN') {
    define('OS_WINDOWS', true);
    define('OS_UNIX', false);
    define('PEAR_OS', 'Windows');
} else {
    define('OS_WINDOWS', false);
    define('OS_UNIX', true);
    define('PEAR_OS', 'Unix'); // blatant assumption
}

$GLOBALS['_PEAR_default_error_mode'] = PEAR_ERROR_RETURN;
$GLOBALS['_PEAR_default_error_options'] = E_USER_NOTICE;
$GLOBALS['_PEAR_default_error_callback'] = '';
$GLOBALS['_PEAR_destructor_object_list'] = array();

//
// Tests needed: - PEAR inheritance
//               - destructors
//

/**
 * Base class for other PEAR classes.  Provides rudimentary
 * emulation of destructors.
 *
 * If you want a destructor in your class, inherit PEAR and make a
 * destructor method called _yourclassname (same name as the
 * constructor, but with a "_" prefix).  Also, in your constructor you
 * have to call the PEAR constructor: <code>$this->PEAR();</code>.
 * The destructor method will be called without parameters.  Note that
 * at in some SAPI implementations (such as Apache), any output during
 * the request shutdown (in which destructors are called) seems to be
 * discarded.  If you need to get any debug information from your
 * destructor, use <code>error_log()</code>, <code>syslog()</code> or
 * something like that instead.
 *
 * @since PHP 4.0.2
 * @author Stig Bakken <ssb@fast.no>
 */
class PEAR
{
    // {{{ properties

    /**
     * Whether to enable internal debug messages.
     *
     * @var     bool
     * @access  private
     */
    var $_debug = false;

    /**
     * Default error mode for this object.
     *
     * @var     int
     * @access  private
     */
    var $_default_error_mode = null;

    /**
     * Default error options used for this object when error mode
     * is PEAR_ERROR_TRIGGER.
     *
     * @var     int
     * @access  private
     */
    var $_default_error_options = null;

    /**
     * Default error handler (callback) for this object, if error mode is
     * PEAR_ERROR_CALLBACK.
     *
     * @var     string
     * @access  private
     */
    var $_default_error_handler = '';

    /**
     * Which class to use for error objects.
     *
     * @var     string
     * @access  private
     */
    var $_error_class = 'PEAR_Error';

    /**
     * An array of expected errors.
     *
     * @var     array
     * @access  private
     */
    var $_expected_errors = null;

    // }}}

    // {{{ constructor

    /**
     * Constructor.  Registers this object in
     * $_PEAR_destructor_object_list for destructor emulation if a
     * destructor object exists.
     *
     * @param string      (optional) which class to use for error objects,
     *                    defaults to PEAR_Error.
     * @access public
     * @return void
     */
    function PEAR($error_class = null)
    {
        $classname = get_class($this);
        if ($this->_debug) {
            print "PEAR constructor called, class=$classname\n";
        }
        if ($error_class !== null) {
            $this->_error_class = $error_class;
        }
        while ($classname) {
            $destructor = "_$classname";
            if (method_exists($this, $destructor)) {
                global $_PEAR_destructor_object_list;
                $_PEAR_destructor_object_list[] = &$this;
                break;
            } else {
                $classname = get_parent_class($classname);
            }
        }
    }

    // }}}
    // {{{ destructor

    /**
     * Destructor (the emulated type of...).  Does nothing right now,
     * but is included for forward compatibility, so subclass
     * destructors should always call it.
     *
     * See the note in the class desciption about output from
     * destructors.
     *
     * @access public
     * @return void
     */
    function _PEAR() {
        if ($this->_debug) {
            printf("PEAR destructor called, class=%s\n",
                   get_class($this));
        }
    }

    // }}}
    // {{{ isError()

    /**
     * Tell whether a value is a PEAR error.
     *
     * @param   mixed   the value to test
     * @access  public
     * @return  bool    true if parameter is an error
     */
    function isError($data) {
        return (bool)(is_object($data) &&
                      (get_class($data) == "pear_error" ||
                       is_subclass_of($data, "pear_error")));
    }

    // }}}
    // {{{ setErrorHandling()

    /**
     * Sets how errors generated by this DB object should be handled.
     * Can be invoked both in objects and statically.  If called
     * statically, setErrorHandling sets the default behaviour for all
     * PEAR objects.  If called in an object, setErrorHandling sets
     * the default behaviour for that object.
     *
     * @param $mode int
     *        one of PEAR_ERROR_RETURN, PEAR_ERROR_PRINT,
     *        PEAR_ERROR_TRIGGER, PEAR_ERROR_DIE or
     *        PEAR_ERROR_CALLBACK.
     *
     * @param $options mixed
     *        Ignored unless $mode is PEAR_ERROR_TRIGGER or
     *        PEAR_ERROR_CALLBACK.  When $mode is PEAR_ERROR_TRIGGER,
     *        this parameter is expected to be an integer and one of
     *        E_USER_NOTICE, E_USER_WARNING or E_USER_ERROR.  When
     *        $mode is PEAR_ERROR_CALLBACK, this parameter is expected
     *        to be the callback function or method.  A callback
     *        function is a string with the name of the function, a
     *        callback method is an array of two elements: the element
     *        at index 0 is the object, and the element at index 1 is
     *        the name of the method to call in the object.
     *
     * @access public
     * @return void
     * @see PEAR_ERROR_RETURN
     * @see PEAR_ERROR_PRINT
     * @see PEAR_ERROR_TRIGGER
     * @see PEAR_ERROR_DIE
     * @see PEAR_ERROR_CALLBACK
     *
     * @since PHP 4.0.5
     */

    function setErrorHandling($mode = null, $options = null)
    {
        if (isset($this)) {
            $setmode = &$this->_default_error_mode;
            $setoptions = &$this->_default_error_options;
            $setcallback = &$this->_default_error_callback;
        } else {
            $setmode = &$GLOBALS['_PEAR_default_error_mode'];
            $setoptions = &$GLOBALS['_PEAR_default_error_options'];
            $setcallback = &$GLOBALS['_PEAR_default_error_callback'];
        }

        switch ($mode) {
            case PEAR_ERROR_RETURN:
            case PEAR_ERROR_PRINT:
            case PEAR_ERROR_TRIGGER:
            case PEAR_ERROR_DIE:
            case null:
                $setmode = $mode;
                $setoptions = $options;
                break;

            case PEAR_ERROR_CALLBACK:
                $setmode = $mode;
                if ((is_string($options) && function_exists($options)) ||
                    (is_array($options) && method_exists(@$options[0], @$options[1])))
                {
                    $setcallback = $options;
                } else {
                    trigger_error("invalid error callback", E_USER_WARNING);
                }
                break;

            default:
                trigger_error("invalid error mode", E_USER_WARNING);
                break;
        }
    }

    // }}}
    // {{{ expectError()

    /**
     * This method is used to tell which errors you expect to get.
     * Expected errors are always returned with error mode
     * PEAR_ERROR_RETURN.  To stop expecting errors, call this method
     * again without parameters.
     *
     * @param mixed    a single error code or an array of error codes
     *                 to expect
     *
     * @return void
     */
    function expectError($code = null)
    {
        if ($code === null || is_array($code)) {
            $this->_expected_errors = $code;
        } else {
            $this->_expected_errors = array($code);
        }
    }

    // }}}
    // {{{ raiseError()

    /**
     * This method is a wrapper that returns an instance of the
     * configured error class with this object's default error
     * handling applied.  If the $mode and $options parameters are not
     * specified, the object's defaults are used.
     *
     * @param $message  a text error message or a PEAR error object
     *
     * @param $code     a numeric error code (it is up to your class
     *                  to define these if you want to use codes)
     *
     * @param $mode     One of PEAR_ERROR_RETURN, PEAR_ERROR_PRINT,
     *                  PEAR_ERROR_TRIGGER, PEAR_ERROR_DIE or
     *                  PEAR_ERROR_CALLBACK.
     *
     * @param $options  If $mode is PEAR_ERROR_TRIGGER, this parameter
     *                  specifies the PHP-internal error level (one of
     *                  E_USER_NOTICE, E_USER_WARNING or E_USER_ERROR).
     *                  If $mode is PEAR_ERROR_CALLBACK, this
     *                  parameter specifies the callback function or
     *                  method.  In other error modes this parameter
     *                  is ignored.
     *
     * @param $userinfo If you need to pass along for example debug
     *                  information, this parameter is meant for that.
     *
     * @param $error_class The returned error object will be instantiated
     *                  from this class, if specified.
     *
     * @param $skipmsg  If true, raiseError will only pass error codes,
     *                  the error message parameter will be dropped.
     *
     * @access public
     * @return object   a PEAR error object
     * @see PEAR::setErrorHandling
     * @since PHP 4.0.5
     */
    function &raiseError($message = null,
                         $code = null,
                         $mode = null,
                         $options = null,
                         $userinfo = null,
                         $error_class = null,
                         $skipmsg = false)
    {
        // The error is yet a PEAR error object
        if (is_object($message)) {
            $code        = $message->getCode();
            $mode        = $message->getMode();
            $userinfo    = $message->getUserInfo();
            $error_class = $message->getType();
            $message     = $message->getMessage();
        }

        if (@in_array($code, $this->_expected_errors)) {
            $mode = PEAR_ERROR_RETURN;
        }

        if ($mode === null) {
            if (isset($this) && isset($this->_default_error_mode)) {
                $mode = $this->_default_error_mode;
            } else {
                $mode = $GLOBALS['_PEAR_default_error_mode'];
            }
        }

        if ($mode == PEAR_ERROR_TRIGGER && $options === null) {
            if (isset($this) && isset($this->_default_error_options)) {
                $options = $this->_default_error_options;
            } else {
                $options = $GLOBALS['_PEAR_default_error_options'];
            }
        }

        if ($mode == PEAR_ERROR_CALLBACK) {
            if (!is_string($options) &&
                !(is_array($options) && sizeof($options) == 2 &&
                  is_object($options[0]) && is_string($options[1]))) {
                if (isset($this) && isset($this->_default_error_callback)) {
                    $options = $this->_default_error_callback;
                } else {
                    $options = $GLOBALS['_PEAR_default_error_callback'];
                }
            }
        } else {
            if ($options === null) {
                if (isset($this) && isset($this->_default_error_options)) {
                    $options = $this->_default_error_options;
                } else {
                    $options = $GLOBALS['_PEAR_default_error_options'];
                }
            }
        }
        if ($error_class !== null) {
            $ec = $error_class;
        } elseif (isset($this) && isset($this->_error_class)) {
            $ec = $this->_error_class;
        } else {
            $ec = 'PEAR_Error';
        }
        if ($skipmsg) {
            return new $ec($code, $mode, $options, $userinfo);
        } else {
            return new $ec($message, $code, $mode, $options, $userinfo);
        }
    }

    // }}}
    // {{{ pushErrorHandling()

    /**
    * Push a new error handler on top of the error handler options stack. With this
    * you can easely override the actual error handler for some code and restore
    * it later with popErrorHandling.
    *
    * @param $mode mixed (same as setErrorHandling)
    * @param $options mixed (same as setErrorHandling)
    *
    * @return bool Always true
    *
    * @see PEAR::setErrorHandling
    */
    function pushErrorHandling($mode, $options = null)
    {
        $stack = &$GLOBALS['_PEAR_error_handler_stack'];
        if (!is_array($stack)) {
            if (isset($this)) {
                $def_mode = &$this->_default_error_mode;
                $def_options = &$this->_default_error_options;
                // XXX Used anywhere?
                //$def_callback = &$this->_default_error_callback;
            } else {
                $def_mode = &$GLOBALS['_PEAR_default_error_mode'];
                $def_options = &$GLOBALS['_PEAR_default_error_options'];
                // XXX Used anywhere?
                //$def_callback = &$GLOBALS['_PEAR_default_error_callback'];
            }
            if (!isset($def_mode)) {
                $def_mode = PEAR_ERROR_RETURN;
            }
            $stack = array();
            $stack[] = array($def_mode, $def_options);
        }

        if (isset($this)) {
            $this->setErrorHandling($mode, $options);
        } else {
            PEAR::setErrorHandling($mode, $options);
        }
        $stack[] = array($mode, $options);
        return true;
    }

    // }}}
    // {{{ popErrorHandling()

    /**
    * Pop the last error handler used
    *
    * @return bool Always true
    *
    * @see PEAR::pushErrorHandling
    */
    function popErrorHandling()
    {
        $stack = &$GLOBALS['_PEAR_error_handler_stack'];
        array_pop($stack);
        list($mode, $options) = $stack[sizeof($stack) - 1];
        if (isset($this)) {
            $this->setErrorHandling($mode, $options);
        } else {
            PEAR::setErrorHandling($mode, $options);
        }
        return true;
    }

    // }}}
}

// {{{ _PEAR_call_destructors()

function _PEAR_call_destructors()
{
    global $_PEAR_destructor_object_list;
    if (is_array($_PEAR_destructor_object_list) &&
        sizeof($_PEAR_destructor_object_list))
    {
        reset($_PEAR_destructor_object_list);
        while (list($k, $objref) = each($_PEAR_destructor_object_list)) {
            $classname = get_class($objref);
            while ($classname) {
                $destructor = "_$classname";
                if (method_exists($objref, $destructor)) {
                    $objref->$destructor();
                    break;
                } else {
                    $classname = get_parent_class($classname);
                }
            }
        }
        // Empty the object list to ensure that destructors are
        // not called more than once.
        $_PEAR_destructor_object_list = array();
    }
}

// }}}

class PEAR_Error
{
    // {{{ properties

    var $error_message_prefix = '';
    var $error_prepend        = '';
    var $error_append         = '';
    var $mode                 = PEAR_ERROR_RETURN;
    var $level                = E_USER_NOTICE;
    var $code                 = -1;
    var $message              = '';
    var $debuginfo            = '';

    // Wait until we have a stack-groping function in PHP.
    //var $file    = '';
    //var $line    = 0;


    // }}}
    // {{{ constructor

    /**
     * PEAR_Error constructor
     *
     * @param $message error message
     *
     * @param $code (optional) error code
     *
     * @param $mode (optional) error mode, one of: PEAR_ERROR_RETURN,
     * PEAR_ERROR_PRINT, PEAR_ERROR_DIE, PEAR_ERROR_TRIGGER or
     * PEAR_ERROR_CALLBACK
     *
     * @param $level (optional) error level, _OR_ in the case of
     * PEAR_ERROR_CALLBACK, the callback function or object/method
     * tuple.
     *
     * @access public
     *
     */
    function PEAR_Error($message = "unknown error", $code = null,
                        $mode = null, $options = null, $userinfo = null)
    {
        if ($mode === null) {
            $mode = PEAR_ERROR_RETURN;
        }
        $this->message   = $message;
        $this->code      = $code;
        $this->mode      = $mode;
        $this->userinfo  = $userinfo;
        if ($mode & PEAR_ERROR_CALLBACK) {
            $this->level = E_USER_NOTICE;
            $this->callback = $options;
        } else {
            if ($options === null) {
                $options = E_USER_NOTICE;
            }
            $this->level = $options;
            $this->callback = null;
        }
        if ($this->mode & PEAR_ERROR_PRINT) {
            print $this->getMessage();
        }
        if ($this->mode & PEAR_ERROR_TRIGGER) {
            trigger_error($this->getMessage(), $this->level);
        }
        if ($this->mode & PEAR_ERROR_DIE) {
            $msg = $this->getMessage();
            if (substr($msg, -1) != "\n") {
                $msg .= "\n";
            }
            die($msg);
        }
        if ($this->mode & PEAR_ERROR_CALLBACK) {
            if (is_string($this->callback) && strlen($this->callback)) {
                call_user_func($this->callback, $this);
            } elseif (is_array($this->callback) &&
                      sizeof($this->callback) == 2 &&
                      is_object($this->callback[0]) &&
                      is_string($this->callback[1]) &&
                      strlen($this->callback[1])) {
                      @call_user_method($this->callback[1], $this->callback[0],
                                 $this);
            }
        }
    }

    // }}}
    // {{{ getMode()

    /**
     * Get the error mode from an error object.
     *
     * @return int error mode
     * @access public
     */
    function getMode() {
        return $this->mode;
    }

    // }}}
    // {{{ getCallback()

    /**
     * Get the callback function/method from an error object.
     *
     * @return mixed callback function or object/method array
     * @access public
     */
    function getCallback() {
        return $this->callback;
    }

    // }}}
    // {{{ getMessage()


    /**
     * Get the error message from an error object.
     *
     * @return  string  full error message
     * @access public
     */
    function getMessage ()
    {
        return ($this->error_prepend . $this->error_message_prefix .
                $this->message       . $this->error_append);
    }


    // }}}
    // {{{ getCode()

    /**
     * Get error code from an error object
     *
     * @return int error code
     * @access public
     */
     function getCode()
     {
        return $this->code;
     }

    // }}}
    // {{{ getType()

    /**
     * Get the name of this error/exception.
     *
     * @return string error/exception name (type)
     * @access public
     */
    function getType ()
    {
        return get_class($this);
    }

    // }}}
    // {{{ getUserInfo()

    /**
     * Get additional user-supplied information.
     *
     * @return string user-supplied information
     * @access public
     */
    function getUserInfo ()
    {
        return $this->userinfo;
    }

    // }}}
    // {{{ getDebugInfo()

    /**
     * Get additional debug information supplied by the application.
     *
     * @return string debug information
     * @access public
     */
    function getDebugInfo ()
    {
        return $this->getUserInfo();
    }

    // }}}
    // {{{ addUserInfo()

    function addUserInfo($info)
    {
        if (empty($this->userinfo)) {
            $this->userinfo = $info;
        } else {
            $this->userinfo .= " ** $info";
        }
    }

    // }}}
    // {{{ toString()

    /**
     * Make a string representation of this object.
     *
     * @return string a string with an object summary
     * @access public
     */
    function toString() {
        $modes = array();
        $levels = array(E_USER_NOTICE => "notice",
                        E_USER_WARNING => "warning",
                        E_USER_ERROR => "error");
        if ($this->mode & PEAR_ERROR_CALLBACK) {
            if (is_array($this->callback)) {
                $callback = get_class($this->callback[0]) . "::" .
                    $this->callback[1];
            } else {
                $callback = $this->callback;
            }
            return sprintf('[%s: message="%s" code=%d mode=callback '.
                           'callback=%s prefix="%s" prepend="%s" append="%s" '.
                           'info="%s"]',
                           get_class($this), $this->message, $this->code,
                           $callback, $this->error_message_prefix,
                           $this->error_prepend, $this->error_append,
                           $this->debuginfo);
        }
        if ($this->mode & PEAR_ERROR_CALLBACK) {
            $modes[] = "callback";
        }
        if ($this->mode & PEAR_ERROR_PRINT) {
            $modes[] = "print";
        }
        if ($this->mode & PEAR_ERROR_TRIGGER) {
            $modes[] = "trigger";
        }
        if ($this->mode & PEAR_ERROR_DIE) {
            $modes[] = "die";
        }
        if ($this->mode & PEAR_ERROR_RETURN) {
            $modes[] = "return";
        }
        return sprintf('[%s: message="%s" code=%d mode=%s level=%s prefix="%s" '.
                       'prepend="%s" append="%s" info="%s"]',
                       get_class($this), $this->message, $this->code,
                       implode("|", $modes), $levels[$this->level],
                       $this->error_message_prefix,
                       $this->error_prepend, $this->error_append,
                       $this->userinfo);
    }

    // }}}
}

register_shutdown_function("_PEAR_call_destructors");

/*
 * Local Variables:
 * mode: c++
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
?>
