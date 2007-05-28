<?php
/** @file phar.php
 * @ingroup Phar
 * @brief class Phar Pre Command
 * @author  Marcus Boerger
 * @date    2007 - 2007
 *
 * Phar Command
 */
foreach(array("SPL", "Reflection", "Phar") as $ext)
{
	if (!extension_loaded($ext))
	{
		echo "$argv[0] requires PHP extension $ext.\n";
		exit(1);
	}
}

if (!class_exists('DirectoryTreeIterator', 0))
{

/** @file directorytreeiterator.inc
 * @ingroup Examples
 * @brief class DirectoryTreeIterator
 * @author  Marcus Boerger
 * @date    2003 - 2005
 *
 * SPL - Standard PHP Library
 */

/** @ingroup Examples
 * @brief   DirectoryIterator to generate ASCII graphic directory trees
 * @author  Marcus Boerger
 * @version 1.1
 */
class DirectoryTreeIterator extends RecursiveIteratorIterator
{
	/** Construct from a path.
	 * @param $path directory to iterate
	 */
	function __construct($path)
	{
		parent::__construct(
			new RecursiveCachingIterator(
				new RecursiveDirectoryIterator($path, RecursiveDirectoryIterator::KEY_AS_FILENAME
				), 
				CachingIterator::CALL_TOSTRING|CachingIterator::CATCH_GET_CHILD
			), 
			parent::SELF_FIRST
		);
	}

	/** @return the current element prefixed with ASCII graphics
	 */	
	function current()
	{
		$tree = '';
		for ($l=0; $l < $this->getDepth(); $l++) {
			$tree .= $this->getSubIterator($l)->hasNext() ? '| ' : '  ';
		}
		return $tree . ($this->getSubIterator($l)->hasNext() ? '|-' : '\-') 
		       . $this->getSubIterator($l)->__toString();
	}

	/** Aggregates the inner iterator
	 */	
	function __call($func, $params)
	{
		return call_user_func_array(array($this->getSubIterator(), $func), $params);
	}
}

}

if (!class_exists('DirectoryGraphIterator', 0))
{

/** @file directorygraphiterator.inc
 * @ingroup Examples
 * @brief class DirectoryGraphIterator
 * @author  Marcus Boerger
 * @date    2003 - 2005
 *
 * SPL - Standard PHP Library
 */

/** @ingroup Examples
 * @brief   A tree iterator that only shows directories.
 * @author  Marcus Boerger
 * @version 1.1
 */
class DirectoryGraphIterator extends DirectoryTreeIterator
{
	function __construct($path)
	{
		RecursiveIteratorIterator::__construct(
			new RecursiveCachingIterator(
				new ParentIterator(
					new RecursiveDirectoryIterator($path, RecursiveDirectoryIterator::KEY_AS_FILENAME
					)
				), 
				CachingIterator::CALL_TOSTRING|CachingIterator::CATCH_GET_CHILD
			), 
			parent::SELF_FIRST
		);
	}
}

}

if (!class_exists('InvertedRegexIterator', 0))
{

/** @file invertedregexiterator.inc
 * @ingroup Phar
 * @brief class InvertedRegexIterator
 * @author  Marcus Boerger
 * @date    2007 - 2007
 *
 * Inverted RegexIterator
 */

/** @ingroup Phar
 * @brief   Inverted RegexIterator
 * @author  Marcus Boerger
 * @version 1.0
 */
class InvertedRegexIterator extends RegexIterator
{
	/** @return !RegexIterator::accept()
	 */	
	function accept()
	{
		return !RegexIterator::accept();
	}
}

}

if (!class_exists('CLICommand', 0))
{

/** @file clicommand.inc
 * @ingroup Phar
 * @brief class CLICommand
 * @author  Marcus Boerger
 * @date    2007 - 2007
 *
 * Phar Command
 */

/** @ingroup Phar
 * @brief   Abstract base console command implementation
 * @author  Marcus Boerger
 * @version 1.0
 */
abstract class CLICommand
{
    protected $argc;
    protected $argv;
    protected $cmds = array();
    protected $args = array();
    protected $typs = array();

    function __construct($argc, array $argv)
    {
        $this->argc = $argc;
        $this->argv = $argv;
        $this->cmds = self::getCommands($this);
        $this->typs = self::getArgTyps($this);

        if ($argc < 2) {
            self::error("No command given, check ${argv[0]} help\n");
        } elseif (!isset($this->cmds[$argv[1]]['run'])) {
            self::error("Unknown command '${argv[1]}', check ${argv[0]} help\n");
        } else {
            $command = $argv[1];
        }

        if (isset($this->cmds[$command]['arg'])) {
            $this->args = call_user_func(array($this, $this->cmds[$command]['arg']));
            $i = 1;
            $missing = false;
            while (++$i < $argc) {
                if ($argv[$i][0] == '-') {
                    if (strlen($argv[$i]) == 2 && isset($this->args[$argv[$i][1]])) {
                        $arg = $argv[$i][1];
                        if (++$i >= $argc) {
                            self::error("Missing argument to parameter '$arg' of command '$command', check ${argv[0]} help\n");
                        } else {
                            $this->args[$arg]['val'] = $this->checkArgTyp($arg, $i, $argc, $argv);
                        }
                    }  else {
                        self::error("Unknown parameter '${argv[$i]}' to command $command, check ${argv[0]} help\n");
                    }
                } else {
                    break;
                }
            }
            if (isset($this->args[''])) {
                if ($i >= $argc) {
                    if (isset($this->args['']['require']) && $this->args['']['require']) {
                        self::error("Missing default trailing arguments to command $command, check ${argv[0]} help\n");
                    }
                } else {
                    $this->args['']['val'] = array();
                    while($i < $argc) {
                        $this->args['']['val'][] = $argv[$i++];
                    }
                }
            } else if ($i < $argc) {
                self::error("Unexpected default arguments to command $command, check ${argv[0]} help\n");
            }
            
            foreach($this->args as $arg => $inf) {
                if (strlen($arg) && !isset($inf['val']) && isset($inf['required']) && $inf['required']) {
                    $missing .=  "Missing parameter '-$arg' to command $command, check ${argv[0]} help\n";
                }
            }
            if (strlen($missing))
            {
                self::error($missing);
            }
        }

        call_user_func(array($this, $this->cmds[$command]['run']), $this->args);
    }

    static function notice ($msg)
    {
        fprintf(STDERR, $msg);
    }

    static function error ($msg, $exit_code = 1) 
    {
        notice($msg);
        exit($exit_code);
    }

    function checkArgTyp($arg, $i, $argc, $argv)
    {
        $typ = $this->args[$arg]['typ'];

        if (isset($this->typs[$typ]['typ'])) {
            return call_user_func(array($this, $this->typs[$typ]['typ']), $argv[$i], $this->args[$arg], $arg);
        } else {
            return $argv[$i];
        }
    }

    static function getSubFuncs(CLICommand $cmdclass, $prefix, array $subs)
    {
        $a = array();
        $r = new ReflectionClass($cmdclass);
        $l = strlen($prefix);

        foreach($r->getMethods() as $m)
        {
            if (substr($m->name, 0, $l) == $prefix)
            {
                foreach($subs as $sub)
                {
                    $what = substr($m->name, $l+strlen($sub)+1);
                    $func = $prefix . $sub . '_' . $what;
                    $what = str_replace('_', '-', $what);
                    if ($r->hasMethod($func))
                    {
                        if (!isset($a[$what]))
                        {
                            $a[$what] = array();
                        }
                        $a[$what][$sub] = /*$m->class . '::' .*/ $func;
                    }
                }
            }
        }
        return $a;
    }

    static function getCommands(CLICommand $cmdclass)
    {
        return self::getSubFuncs($cmdclass, 'cli_cmd_', array('arg','inf','run'));
    }

    static function getArgTyps(CLICommand $cmdclass)
    {
        return self::getSubFuncs($cmdclass, 'cli_arg_', array('typ'));
    }

    static function cli_arg_typ_bool($arg, $cfg, $key)
    {
        return (bool)$arg;
    }

    
    static function cli_arg_typ_int($arg, $cfg, $key) 
    {
        if ((int)$arg != $arg) {
            self::error("Argument to -$key must be an integer.\n");
        }

        return (int)$arg;
    }
    
    static function cli_arg_typ_regex($arg, $cfg, $key)
    {
        if (strlen($arg))
        {
            if (strlen($arg) > 1 && $arg[0] == $arg[strlen($arg)-1] && strpos('/,', $arg) !== false)
            {
                return $arg;
            }
            else
            {
                return '/' . $arg . '/';
            }
        }
        else
        {
            return NULL;
        }
    }

    static function cli_arg_typ_select($arg, $cfg, $key)
    {
        if (!in_array($arg, array_keys($cfg['select']))) {
            self::error("Parameter value '$arg' not one of '" . join("', '", array_keys($cfg['select'])) . "'.\n");
        }
        return $arg;
    }

    static function cli_arg_typ_dir($arg, $cfg, $key)
    {
        $f = realpath($arg);

        if ($f===false || !file_exists($f) || !is_dir($f)) {
            self::error("Requested path '$arg' does not exist.\n");
        }
        return $f;
    }

    static function cli_arg_typ_file($arg)
    {
        $f = new SplFileInfo($arg);
        $f = $f->getRealPath();
        if ($f===false || !file_exists($f))
        {
            echo "Requested file '$arg' does not exist.\n";
            exit(1);
        }
        return $f;
    }

    static function cli_arg_typ_filenew($arg, $cfg, $key)
    {
        $d = dirname($arg);
        $f = realpath($d);
        
        if ($f === false) {
            self::error("Path for file '$arg' does not exist.\n");
        }
        return $f . substr($arg, strlen($d));;
    }

    static function cli_arg_typ_filecont($arg, $cfg, $key)
    {
        return file_get_contents(self::cli_arg_typ_file($arg, $cfg, $key));
    }

    function cli_get_SP2($l1, $arg_inf)
    {
        return str_repeat(' ', $l1 + 2 + 4 + 8);
    }

    function cli_get_SP3($l1, $l2, $arg_inf)
    {
        return str_repeat(' ', $l1 + 2 + 4 + 8 + 2 + $l2 + 2);
    }

    static function cli_cmd_inf_help()
    {
        return "This help or help for a selected command.";
    }

    private function cli_wordwrap($what, $l, $sp)
    {
        $p = max(79 - $l, 40);     // minimum length for paragraph
        $b = substr($what, 0, $l); // strip out initial $l
        $r = substr($what, $l);    // remainder
        $r = str_replace("\n", "\n".$sp, $r); // in remainder replace \n's
        return $b . wordwrap($r, $p, "\n".$sp);
    }

    private function cli_help_get_args($func, $l, $sp, $required)
    {
        $inf = "";
        foreach(call_user_func($func, $l, $sp) as $arg => $conf)
        {
            if ((isset($conf['required']) && $conf['required']) != $required)
            {
                continue;
            }
            if (strlen($arg))
            {
                $arg = "-$arg  ";
            }
            else
            {
                $arg = "... ";
            }
            $sp2 = $this->cli_get_SP2($l, $inf);
            $l2  = strlen($sp2);
            $inf .= $this->cli_wordwrap($sp . $arg . $conf['inf'], $l2, $sp2) . "\n";
            if (isset($conf['select']) && count($conf['select']))
            {
                $ls = 0;
                foreach($conf['select'] as $opt => $what)
                {
                    $ls = max($ls, strlen($opt));
                }
                $sp3 = $this->cli_get_SP3($l, $ls, $inf);
                $l3  = strlen($sp3);
                foreach($conf['select'] as $opt => $what)
                {
                    $inf .= $this->cli_wordwrap($sp2 . "  " . sprintf("%-${ls}s  ", $opt) . $what, $l3, $sp3) . "\n";
                }
            }
        }
        if (strlen($inf))
        {
            if ($required)
            {
                return $sp . "Required arguments:\n\n" . $inf;
            }
            else
            {
                return $sp . "Optional arguments:\n\n". $inf;
            }
        }
    }

    function cli_cmd_arg_help()
    {
        return array('' => array('typ'=>'any','val'=>NULL,'inf'=>'Optional command to retrieve help for.'));
    }

    function cli_cmd_run_help()
    {
        $argv  = $this->argv;
        $which = $this->args['']['val'];
        if (isset($which))
        {
            if (count($which) != 1) {
                self::error("More than one command given.\n");
            }
            
            $which = $which[0];
            if (!array_key_exists($which, $this->cmds)) {
                self::error("Unknown command, cannot retrieve help.\n");
            }

            $l = strlen($which);
            $cmds = array($which => $this->cmds[$which]);
        } else {
            echo "\n$argv[0] <command> [options]\n\n";
            $l = 0;
            ksort($this->cmds);
            foreach($this->cmds as $name => $funcs) {
                $l = max($l, strlen($name));
            }
            $inf = "Commands:";
            $lst = "";
            $ind = strlen($inf) + 1;
            foreach($this->cmds as $name => $funcs)
            {
                $lst .= ' ' . $name;
            }
            echo $this->cli_wordwrap($inf.$lst, $ind, str_repeat(' ', $ind)) . "\n\n";
            $cmds = $this->cmds;
        }
        $sp = str_repeat(' ', $l + 2);
        foreach($cmds as $name => $funcs)
        {
            $inf = $name . substr($sp, strlen($name));
            if (isset($funcs['inf']))
            {
                $inf .= $this->cli_wordwrap(call_user_func(array($this, $funcs['inf'])), $l, $sp) . "\n";
                if (isset($funcs['arg']))
                {
                    $inf .= "\n";
                    $inf .= $this->cli_help_get_args(array($this, $funcs['arg']), $l, $sp, true);
                    $inf .= "\n";
                    $inf .= $this->cli_help_get_args(array($this, $funcs['arg']), $l, $sp, false);
                }
            }
            echo "$inf\n\n";
        }
        exit(0);
    }

    static function cli_cmd_inf_help_list()
    {
        return "Lists available commands.";
    }

    function cli_cmd_run_help_list()
    {
        ksort($this->cmds);
        $lst = '';
        foreach($this->cmds as $name => $funcs) {
            $lst .= $name . ' ';
        }
        echo substr($lst, 0, -1) . "\n";
    }
}

}

if (!class_exists('PharCommand', 0))
{

/**
 * @file pharcommand.inc
 * @ingroup Phar
 * @brief class CLICommand
 * @author  Marcus Boerger
 * @date    2007 - 2007
 *
 * Phar Command
 */
// {{{ class PharCommand extends CLICommand
/**
 * PharCommand class
 * 
 * This class handles the handling of the phar
 * commands. It will be used from command line/console
 * in order to retrieve and execute phar functions.
 * 
 * @ingroup Phar
 * @brief   Phar console command implementation
 * @author  Marcus Boerger
 * @version 1.0
 */
class PharCommand extends CLICommand
{
    // {{{ public function cli_get_SP2
    public function cli_get_SP2($l1, $arg_inf)
    {
        return str_repeat(' ', $l1 + 2 + 4 + 9);
    }
    // }}}
    // {{{ public function cli_get_SP3
    /**
     * Cli Get SP3
     *
     * @param string $l1      Eleven
     * @param string $l2      Twelve
     * @param string $arg_inf 
     * @return string  The repeated string.
     */
    function cli_get_SP3($l1, $l2, $arg_inf)
    {
        return str_repeat(' ', $l1 + 2 + 4 + 9 + 2 + $l2 + 2);
    }
    // }}}
    // {{{ static function phar_args
    /**
     * Phar arguments
     * 
     * This function contains all the phar commands
     *
     * @param  string $which    Which argument is chosen.
     * @param  string $phartype The type of phar, specific file to work on
     * @return unknown
     */
    static function phar_args($which, $phartype)
    {
        $phar_args = array(
            'a' => array(
                'typ' => 'alias',
                'val' => NULL,
                'inf' => '<alias>  Provide an alias name for the phar file.'
            ),
            'c' => array(
                'typ' => 'compalg',
                'val' => NULL,
                'inf' => '<algo>   Compression algorithm.',
                'select' => array(
                    '0'    => 'No compression',
                    'none' => 'No compression',
                    'auto' => 'Automatically select compression algorithm'
                )
            ),
            'e' => array(
                'typ' => 'entry',
                'val' => NULL,
                'inf' => '<entry>  Name of entry to work on (must include PHAR internal directory name if any).'
            ),
            'f' => array(
                'typ' => $phartype,
                'val' => NULL,
                'inf' => '<file>   Specifies the phar file to work on.'
            ),
            'h' => array(
                'typ' => 'select',
                'val' => NULL,
                'inf' => '<method> Selects the hash algorithmn.',
                'select' => array('md5' => 'MD5','sha1' => 'SHA1')
            ),
            'i' => array(
                'typ' => 'regex',
                'val' => NULL,
                'inf' => '<regex>  Specifies a regular expression for input files.'
            ),
            'k' => array(
                'typ' => 'any',
                'val' => NULL,
                'inf' => '<index>  Subscription index to work on.',
            ),
            'l' => array(
                'typ' => 'int',
                'val' => 0,
                'inf' => '<level>  Number of preceeding subdirectories to strip from file entries',
            ),
            'm' => array(
                'typ' => 'any',
                'val' => NULL,
                'inf' => '<meta>   Meta data to store with entry (serialized php data).'
            ),
            'p' => array(
                'typ' => 'loader',
                'val' => NULL,
                'inf' => '<loader> Location of PHP_Archive class file (pear list-files PHP_Archive).'
                         .'You can use \'0\' or \'1\' to locate it automatically using the mentioned '
                         .'pear command. When using \'0\' the command does not error out when the '
                         .'class file cannot be located. This switch also adds some code around the '
                         .'stub so that class PHP_Archive gets registered as phar:// stream wrapper '
                         .'if necessary. And finally this switch will add the file phar.inc from '
                         .'this package and load it to ensure class Phar is present.'
                         ,
            ),
            's' => array(
                'typ' => 'file',
                'val' => NULL,
                'inf' => '<stub>   Select the stub file.'
            ),
            'x' => array(
                'typ' => 'regex',
                'val' => NULL,
                'inf' => '<regex>  Regular expression for input files to exclude.'
            ),

        );

        if (extension_loaded('zlib')) {
            $phar_args['c']['select']['gz']    = 'GZip compression';
            $phar_args['c']['select']['gzip']  = 'GZip compression';
        }

        if (extension_loaded('bz2')) {
            $phar_args['c']['select']['bz2']   = 'BZip2 compression';
            $phar_args['c']['select']['bzip2'] = 'BZip2 compression';
        }

        $hash_avail = Phar::getSupportedSignatures();
        if (in_array('SHA-256', $hash_avail)) {
            $phar_args['h']['select']['sha256'] = 'SHA256';
        }

        if (in_array('SHA-512', Phar::getSupportedSignatures())) {
            $phar_args['h']['select']['sha512'] = 'SHA512';
        }

        $args = array();

        foreach($phar_args as $lkey => $cfg) {
            $ukey     = strtoupper($lkey);
            $required = strpos($which, $ukey) !== false;
            $optional = strpos($which, $lkey) !== false;

            if ($required || $optional) {
                $args[$lkey] = $cfg;
                $args[$lkey]['required'] = $required;
            }
        }
        return $args;
    }
    // }}}
    // {{{ static function strEndsWith
    /**
     * String Ends With
     * 
     * Wether a string end with another needle.
     *
     * @param string $haystack  The haystack
     * @param string $needle    The needle.
     * @return mixed false if doesn't end with anything, the string 
     *               substr'ed if the string ends with the needle.
     */
    static function strEndsWith($haystack, $needle)
    {
        return substr($haystack, -strlen($needle)) == $needle;
    }
    // }}}
    // {{{ static function cli_arg_typ_loader
    /**
     * Argument type loader
     *
     * @param string $arg   Either 'auto', 'optional' or an filename that 
     *                      contains class PHP_Archive
     * @param  string $cfg  Configuration to pass to a new file
     * @param  string $key  The key 
     * @return string $arg  The argument.
     */
    static function cli_arg_typ_loader($arg, $cfg, $key)
    {
        if (($arg == '0' || $arg == '1') && !file_exists($arg)) {
            $found = NULL;
            foreach(split("\n", `pear list-files PHP_Archive`) as $ent) {
                $matches = NULL;
                if (preg_match(",^php[ \t]+([^ \t].*pear[\\\\/]PHP[\\\\/]Archive.php)$,", $ent, $matches)) {
                    $found = $matches[1];
                    break;
                }
            }
            if (!isset($found)) {
            	$msg = "Pear package PHP_Archive or Archive.php class file not found.\n";
            	if ($arg == '0') {
            		self::notice($msg);
            	} else {
            		self::error($msg);
            	}
            }
            $arg = $found;
        }
        return self::cli_arg_typ_file($arg);
    }
    // }}}
    // {{{ static function cli_arg_typ_pharnew
    /**
     * Argument type new phar
     *
     * @param  string $arg  The new phar component.
     * @param  string $cfg  Configuration to pass to a new file
     * @param  string $key  The key 
     * @return string $arg  The new argument file.
     */
    static function cli_arg_typ_pharnew($arg, $cfg, $key)
    {
        $arg = self::cli_arg_typ_filenew($arg, $cfg, $key);
        if (!Phar::isValidPharFilename($arg)) {
            self::error("Phar files must have file extension '.phar', '.phar.php', '.phar.bz2' or 'phar.gz'.\n");
        }
        return $arg;
    }
    // }}}
    // {{{ static function cli_arg_typ_pharfile
    /**
     * Argument type existing Phar file
     * 
     * Return filenam eof an existing Phar.
     *
     * @param  string $arg      The file in the phar to open.
     * @param  string $cfg      The configuration information
     * @param  string $key      The key information.
     * @return string $pharfile The name of the loaded Phar file.
     * @note The Phar will be loaded
     */
    static function cli_arg_typ_pharfile($arg, $cfg, $key)
    {
        try {
            $pharfile = self::cli_arg_typ_file($arg, $cfg, $key);

            if (!Phar::loadPhar($pharfile)) {
                self::error("Unable to open phar '$arg'\n");
            }

            return $pharfile;
        } catch(Exception $e) {
            self::error("Exception while opening phar '$arg':\n" . $e->getMessage() . "\n");
        }
    }
    // }}}
    // {{{ static function cli_arg_typ_pharurl
    /**
     * Argument type Phar url-like
     * 
     * Check the argument as cli_arg_Typ_phar and return its name prefixed 
     * with phar://
     * 
     * Ex:
     * <code>
     *  $arg = 'pharchive.phar/file.php';
     *  cli_arg_typ_pharurl($arg)
     * </code>
     *
     * @param  string $arg The url-like phar archive to retrieve.
     * @return string The phar file-archive.
     */
    static function cli_arg_typ_pharurl($arg, $cfg, $key)
    {
        return 'phar://' . self::cli_arg_typ_pharfile($arg, $cfg, $key);
    }
    // }}}
    // {{{ static function cli_arg_typ_phar
    /**
     * Cli argument type phar
     *
     * @param  string $arg  The phar archive to use.
     * @return object new Phar of the passed argument.
     */
    static function cli_arg_typ_phar($arg, $cfg, $key)
    {
        try {
            return new Phar(self::cli_arg_typ_pharfile($arg, $cfg, $key));
        } catch(Exception $e) {
            self::error("Exception while opening phar '$argv':\n" . $e->getMessage() . "\n");
        }
    }
    // }}}
    // {{{ static function cli_arg_typ_entry
    /**
     * Argument type Entry name
     *
     * @param  string $arg The argument (the entry)
     * @return string $arg The entry itself.
     */
    static function cli_arg_typ_entry($arg, $cfg, $key)
    {
        // no further check atm, maybe check for no '/' at beginning
        return $arg;
    }
    // }}}
    // {{{ static function cli_arg_typ_compalg
    /**
     * Argument type compression algorithm
     *
     * @param  string $arg  The phar selection
     * @param  string $cfg  The config option.
     * @param  string $key  The key information.
     * @return string $arg  The selected algorithm
     */
    static function cli_arg_typ_compalg($arg, $cfg, $key)
    {
        $arg = self::cli_arg_typ_select($arg, $cfg, $key);
        
        switch($arg) {
            case 'auto':
                if (extension_loaded('zlib')) {
                    $arg = 'gz';
                } elseif (extension_loaded('bz2')) {
                    $arg = 'bz2';
                } else {
                    $arg = '0';
                }
                break;
        }
        return $arg;
    }
    // }}}
    // {{{ static function cli_cmd_inf_pack
    /**
     * Information pack
     *
     * @return string A description about packing files into a Phar archive.
     */
    static function cli_cmd_inf_pack()
    {
        return "Pack files into a PHAR archive.\n" .  
               "When using -s <stub>, then the stub file is being " .
               "excluded from the list of input files/dirs." .
               "To create an archive that contains PEAR class PHP_Archiave " .
               "then point -p argument to PHP/Archive.php.\n";
    }
    // }}}
    // {{{ static function cli_cmd_arg_pack
    /**
     * Pack a new phar infos
     *
     * @return array  $args  The arguments for a new Phar archive.
     */
    static function cli_cmd_arg_pack()
    {
        $args = self::phar_args('acFhilpsx', 'pharnew');
        
        $args[''] = array(
            'typ'     => 'any',     
            'val'      => NULL,      
            'required' => 1, 
            'inf'      => '         Any number of input files and directories. If -i is in use then ONLY files and matching thegiven regular expression are being packed. If -x is given then files matching that regular expression are NOT being packed.',
            
        );
        
        return $args;
    }
    // }}}
    // {{{ function cli_cmd_run_pack
    /**
     * Pack a new Phar
     * 
     * This function will try to pack a new Phar archive.
     * 
     * @see Exit to make sure that we are done.
     */
    public function cli_cmd_run_pack()
    {
        if (ini_get('phar.readonly')) {
            self::error("Creating phar files is disabled by ini setting 'phar.readonly'.\n");
        }
        
        if (!Phar::canWrite()) {
            self::error("Creating phar files is disabled, Phar::canWrite() returned false.\n");
        }

        $alias    = $this->args['a']['val'];
        $archive  = $this->args['f']['val'];
        $hash     = $this->args['h']['val'];
        $regex    = $this->args['i']['val'];
        $level    = $this->args['l']['val'];
        $loader   = $this->args['p']['val'];
        $stub     = $this->args['s']['val'];
        $invregex = $this->args['x']['val'];
        $input    = $this->args['']['val'];

        $phar  = new Phar($archive, 0, $alias);

        $phar->startBuffering();

        if (isset($stub)) {
            if (isset($loader)) {
                $c = file_get_contents($stub);
                $s = '';

                if (substr($c, 0, 2) == '#!') {
                    $s .= substr($c, 0, strpos($c, "\n") + 1);
                }

				$s .= "<?php if (!class_exists('PHP_Archive')) {\n?>";
				$s .= file_get_contents($loader);
                $s .= "<?php\n";
                $s .= "}\n";
                $s .= "if (!in_array('phar', stream_get_wrappers())) {\n\tstream_wrapper_register('phar', 'PHP_Archive');\n}\n";
				$s .= "if (!class_exists('Phar',0)) {\n";
                $s .= "\tinclude 'phar://'.__FILE__.'/phar.inc';\n";
                $s .= "}\n";
                $s .= '?>';

                if (substr($c,0,1) == '#') {
                    $s.= substr($c,strpos($c, "\n")+1);
                }

                $phar->setStub($s);
            } else {
                $phar->setStub(file_get_contents($stub));
            }
            $stub = new SplFileInfo($stub);
        }

        if (!is_array($input)) {
            $this->phar_add($phar, $level, $input, $regex, $invregex, $stub, NULL, isset($loader));
        } else {
            foreach($input as $i) {
                $this->phar_add($phar, $level, $i, $regex, $invregex, $stub, NULL, isset($loader));
            }
        }

        if (isset($loader)) {
        	if (substr(__FILE__, -15) == 'pharcommand.inc') {
	            self::phar_add_file($phar, 0, 'phar.inc', 'phar://'.__FILE__.'/phar.inc', NULL);
	        } else {
	            self::phar_add_file($phar, 0, 'phar.inc', dirname(__FILE__).'/phar/phar.inc', NULL);
	        }
        }

        switch($this->args['c']['val']) {
            case 'gz':
            case 'gzip':
                $phar->compressAllFilesGZ();
                break;
            case 'bz2':
            case 'bzip2':
                $phar->compressAllFilesBZIP2();
                break;
            default:
                $phar->uncompressAllFiles();
                break;
        }

        if ($hash) {
            $phar->setSignatureAlgorithm($hash);
        }

        $phar->stopBuffering();
        exit(0);
    }
    // }}}
    // {{{ static function phar_add
    /**
     * Add files to a phar archive.
     *
     * This function will take a directory and iterate through
     * it and get the files to insert into the Phar archive.
     * 
     * @param Phar        $phar      The phar object.
     * @param string      $input     The input directory
     * @param string      $regex     The regex use in RegexIterator.
     * @param string      $invregex  The InvertedRegexIterator expression.
     * @param SplFileInfo $stub Stub file object    
     * @param mixed       $compress  Compression algorithm or NULL
     * @param boolean     $noloader  Whether to prevent adding the loader
     */
    static function phar_add(Phar $phar, $level, $input, $regex, $invregex, SplFileInfo $stub = NULL, $compress = NULL, $noloader = false)
    {
        if ($input && is_file($input) && !is_dir($input)) {
            return self::phar_add_file($phar, $level, $input, $input, $compress);
        }
        $dir   = new RecursiveDirectoryIterator($input);
        $dir   = new RecursiveIteratorIterator($dir);

        if (isset($regex)) {
            $dir = new RegexIterator($dir, $regex);
        }

        if (isset($invregex)) {
            $dir = new InvertedRegexIterator($dir, $invregex);
        }

        try {
            foreach($dir as $file) {
                if (empty($stub) || $file->getRealPath() != $stub->getRealPath()) {
                    self::phar_add_file($phar, $level, $dir->getSubPathName(), $file, $compress, $noloader);
                }
            }
        } catch(Excpetion $e) {
            self::error("Unable to complete operation on file '$file'\n" . $e->getMessage() . "\n");
        }
    }
    // }}}
    // {{{ static function phar_add_file
    /**
     * Add a phar file
     *
     * This function adds a file to a phar archive.
     *
     * @param Phar    $phar      The phar object
     * @param string  $level     The level of the file.
     * @param string  $entry     The entry point
     * @param string  $file      The file to add to the archive
     * @param string  $compress  The compression scheme for the file.
     * @param boolean $noloader  Whether to prevent adding the loader
     */
    static function phar_add_file(Phar $phar, $level, $entry, $file, $compress, $noloader = false)
    {
        $entry = str_replace('//', '/', $entry);
        while($level-- > 0 && ($p = strpos($entry, '/')) !== false) {
            $entry = substr($entry, $p+1);
        }

		if ($noloader && $entry == 'phar.inc') {
			return;
		}

        echo "$entry\n";

        $phar[$entry] = file_get_contents($file);
        switch($compress) {
            case 'gz':
            case 'gzip':
                $phar[$entry]->setCompressedGZ();
                break;
            case 'bz2':
            case 'bzip2':
                $phar[$entry]->setCompressedBZIP2();
                break;
            default:
                break;
        }
    }
    // }}}
    // {{{ public function phar_dir_echo
    /**
     * Echo directory
     *
     * @param string $pn      
     * @param unknown_type $f
     */
    public function phar_dir_echo($pn, $f)
    {
        echo "$f\n";
    }
    // }}}
    // {{{ public function phar_dir_operation
    /**
     * Directory operations
     * 
     * Phar directory operations.
     *
     * @param RecursiveIteratorIterator $dir  The recursiveIteratorIterator object.
     * @param string                    $func Function to call on the iterations
     * @param array                     $args Function arguments.
     */
    public function phar_dir_operation(RecursiveIteratorIterator $dir, $func, array $args = array())
    {
        $regex   = $this->args['i']['val'];
        $invregex= $this->args['x']['val'];

        if (isset($regex)) {
            $dir = new RegexIterator($dir, $regex);
        }

        if (isset($invregex)) {
            $dir = new InvertedRegexIterator($dir, $invregex);
        }

        foreach($dir as $pn => $f) {
            call_user_func($func, $pn, $f, $args);
        }
    }
    // {{{ static function cli_cmd_inf_list
    /**
     * Cli Command Info List
     *
     * @return string What inf does
     */
    static function cli_cmd_inf_list()
    {
        return "List contents of a PHAR archive.";
    }
    // }}}
    // {{{ static function cli_cmd_arg_list
    /**
     * Cli Command Argument List
     *
     * @return arguments list
     */
    static function cli_cmd_arg_list()
    {
        return self::phar_args('Fix', 'pharurl');
    }
    // }}}
    // {{{ public function cli_cmd_run_list
    /**
     * Cli Command Run List
     *
     * @see $this->phar_dir_operation
     */
    public function cli_cmd_run_list()
    {
        $this->phar_dir_operation(
            new DirectoryTreeIterator(
                $this->args['f']['val']), 
                array($this, 'phar_dir_echo')
            );
    }
    // }}}
    // {{{ static function cli_command_inf_tree
    /**
     * Cli Command Inf Tree
     *
     * @return string  The description of a directory tree for a Phar archive.
     */
    static function cli_cmd_inf_tree()
    {
        return "Get a directory tree for a PHAR archive.";
    }
    // }}}
    // {{{ static function cli_cmd_arg_tree
    /**
     * Cli Command Argument Tree
     *
     * @return string Arguments in URL format.
     */
    static function cli_cmd_arg_tree()
    {
        return self::phar_args('Fix', 'pharurl');
    }
    // }}}
    // {{{ public function cli_cmd_run_tree
    /**
     * Cli Command Run Tree
     * 
     * Set the phar_dir_operation with a directorygraphiterator.
     * 
     * @see DirectoryGraphIterator
     * @see $this->phar_dir_operation
     *
     */
    public function cli_cmd_run_tree()
    {
        $this->phar_dir_operation(
            new DirectoryGraphIterator(
                $this->args['f']['val']), 
                array($this, 'phar_dir_echo')
            );
    }
    // }}}
    // {{{ cli_cmd_inf_extract
    /**
     * Cli Command Inf Extract
     *
     * @return string The description of the command extra to a directory.
     */
    static function cli_cmd_inf_extract()
    {
        return "Extract a PHAR package to a directory.";
    }
    // }}}
    // {{{ static function cli_cmd_arg_extract
    /**
     * Cli Command Arguments Extract
     * 
     * The arguments for the extract function.
     *
     * @return array  The arguments for the extraction.
     */
    static function cli_cmd_arg_extract()
    {
        $args = self::phar_args('Fix', 'phar');
        
        $args[''] = array(
            'type' => 'dir',  
            'val' => '.',                 
            'inf' => '         Directory to extract to (defaults to \'.\').',
        );
        
        return $args;
    }
    // }}}
    // {{{ public function cli_cmd_run_extract
    /**
     * Run Extract
     * 
     * Run the extraction of a phar Archive.
     *
     * @see $this->phar_dir_operation
     */
    public function cli_cmd_run_extract()
    {
        $dir = $this->args['']['val'];
        
        if (is_array($dir)) {
            if (count($dir) != 1) {
                self::error("Only one target directory allowed.\n");
            } else {
                $dir = $dir[0];
            }
        }
        
        $phar = $args['f']['val'];
        $base = $phar->getPathname();
        $bend = strpos($base, '.phar');
        $bend = strpos($base, '/', $bend);
        $base = substr($base, 0, $bend + 1);
        $blen = strlen($base);

        $this->phar_dir_operation(
            new RecursiveIteratorIterator($phar), 
            array($this, 'phar_dir_extract'), 
            array($blen, $dir)
        );
    }
    // }}}
    // {{{ public function phar_dir_extract
    /**
     * Extract to a directory
     * 
     * This function will extract the content of a Phar
     * to a directory and create new files and directories
     * depending on the permissions on that folder.
     *
     * @param string $pn
     * @param string $f     The file name
     * @param array $args   The directory and Blen informations
     */
    public function phar_dir_extract($pn, $f, $args)
    {
        $blen   = $args[0];
        $dir    = $args[1];
        $sub    = substr($pn, $blen);
        $target = $dir . '/' . $sub;
        
        if (!file_exists(dirname($target))) {
            if (!is_writable(dirname($target))) {
                self::error("Operation could not be completed\n");
            }
            
            mkdir(dirname($target));
        }
        
        echo "$sub";
        
        if (!@copy($f, $target)) {
            echo " ...error\n";
        } else {
            echo " ...ok\n";
        }
    }
    // }}}
    // {{{ static function cli_cmd_inf_delete
    /**
     * Delete an entry from a phar information.
     *
     * @return string The information
     */
    static function cli_cmd_inf_delete()
    {
        return 'Delete entry from a PHAR archive';
    }
    // }}}
    // {{{ static function cli_cmd_arg_delete
    /**
     * The cli command argument for deleting.
     *
     * @return array informations about the arguments to use.
     */
    static function cli_cmd_arg_delete()
    {
        return self::phar_args('FE', 'phar');
    }
    // }}}
    // {{{ public function cli_cmd_run_delete
    /**
     * Deleting execution
     *
     * Execute the deleting of the file from the phar archive.
     */
    public function cli_cmd_run_delete()
    {
        $phar  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];

       $phar->startBuffering();
       unset($phar[$entry]);
       $phar->stopBuffering();
    }
    // }}}
    // {{{ static function cli_cmd_inf_add
    /**
     * Client comment add file information
     *
     * @return string The description of the feature
     */
    static function cli_cmd_inf_add()
    {
        return "Add entries to a PHAR package.";
    }
    // }}}
    // {{{ static function cli_cmd_arg_add
    /**
     * Add a file arguments
     */
    static function cli_cmd_arg_add()
    {
        $args = self::phar_args('acFilx', 'phar');
        $args[''] = array(
            'type'     => 'any',     
            'val'      => NULL,      
            'required' => 1, 
            'inf'      => '         Any number of input files and directories. If -i is in use then ONLY files and matching thegiven regular expression are being packed. If -x is given then files matching that regular expression are NOT being packed.',
        );
        return $args;
    }
    // }}}
    // {{{ public functio cli_cmd_run_add
    /**
     * Add a file
     *
     * Run the action of adding a file to
     * a phar archive.
     */
    public function cli_cmd_run_add()
    {
        $compress= $this->args['c']['val'];
        $phar    = $this->args['f']['val'];
        $regex   = $this->args['i']['val'];
        $level   = $this->args['l']['val'];
        $invregex= $this->args['x']['val'];
        $input   = $this->args['']['val'];

        $phar->startBuffering();

        if (!is_array($input)) {
            $this->phar_add($phar, $level, $input, $regex, $invregex, NULL, $compress);
        } else {
            foreach($input as $i) {
                $this->phar_add($phar, $level, $i, $regex, $invregex, NULL, $compress);
            }
        }
        $phar->stopBuffering();
        exit(0);
    }
    // }}}
    // {{{ public function cli_cmd_inf_stub_set
    /**
     * Set the stup of a phar file.
     *
     * @return string The stub set description.
     */
    public function cli_cmd_inf_stub_set()
    {
        return "Set the stub of a PHAR file. " . 
               "If no input file is specified as stub then stdin is being used.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_stub_set
    /**
     * Set the argument stub
     *
     * @return string arguments for a stub 
     */
    public function cli_cmd_arg_stub_set()
    {
        $args = self::phar_args('Fs', 'phar');
        $args['s']['val'] = 'php://stdin';
        return $args;
    }
    // }}}
    // {{{ public function cli_cmd_run_stub_set
    /**
     * Cli Command run stub set
     *
     * @see   $phar->setStub()
     */
    public function cli_cmd_run_stub_set()
    {
        $phar = $this->args['f']['val'];
        $stub = $this->args['s']['val'];

        $phar->setStub(file_get_contents($stub));
    }
    // }}}
    // {{{ public function cli_cmd_inf_stub_get
    /**
     * Get the command stub infos.
     *
     * @return string a description of the stub of a Phar file.
     */
    public function cli_cmd_inf_stub_get()
    {
        return "Get the stub of a PHAR file. " .  
               "If no output file is specified as stub then stdout is being used.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_stub_get
    /**
     * Get the argument stub
     *
     * @return array $args The arguments passed to the stub.
     */
    public function cli_cmd_arg_stub_get()
    {
        $args = self::phar_args('Fs', 'phar');
        $args['s']['val'] = 'php://stdin';
        return $args;
    }
    // }}}
    // {{{ public function cli_cmd_run_stub_get
    /**
     * Cli Command Run Stub
     * 
     * Get arguments and store them into a stub.
     *
     * @param arguments $args
     * @see   $this->args
     */
    public function cli_cmd_run_stub_get($args)
    {
        $phar = $this->args['f']['val'];
        $stub = $this->args['s']['val'];

        file_put_contents($stub, $phar->getStub());
    }
    // }}}
    // {{{ public function cli_cmd_inf_compress
    /**
     * Cli Command Inf Compress
     * 
     * Cli Command compress informations
     *
     * @return string A description of the command.
     */
    public function cli_cmd_inf_compress()
    {
        return "Compress or uncompress all files or a selected entry.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_cmpress
    /**
     * Cli Command Arg Compress
     *
     * @return array The arguments for compress
     */
    public function cli_cmd_arg_compress()
    {
        return self::phar_args('FCe', 'phar');
    }
    // }}}
    // {{{ public function cli_cmd_run_compress
    /**
     * Cli Command Run Compress
     *
     * @see $this->args
     */
    public function cli_cmd_run_compress()
    {
        $phar  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];

        switch($this->args['c']['val']) {
            case 'gz':
            case 'gzip':
                if (isset($entry)) {
                    $phar[$entry]->setCompressedGZ();
                } else {
                    $phar->compressAllFilesGZ();
                }
                break;
            case 'bz2':
            case 'bzip2':
                if (isset($entry)) {
                    $phar[$entry]->setCompressedBZIP2();
                } else {
                    $phar->compressAllFilesBZIP2();
                }
                break;
            default:
                if (isset($entry)) {
                    $phar[$entry]->setUncompressed();
                } else {
                    $phar->uncompressAllFiles();
                }
                break;
        }
    }
    // }}}
    // {{{ public function cli_cmd_inf_sign
    /**
     * Cli Command Info Signature
     *
     * @return string A description of the signature arguments.
     */
    public function cli_cmd_inf_sign()
    {
        return "Set signature hash algorithm.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_sign
    /**
     * Cli Command Argument Sign
     *
     * @return array Arguments for Signature
     */
    public function cli_cmd_arg_sign()
    {
        return self::phar_args('FH', 'phar');
    }
    // }}}
    // {{{ public function cli_cmd_run_sign
    /**
     * Cli Command Run Signature
     *
     * @see $phar->setSignaturealgorithm
     */
    public function cli_cmd_run_sign()
    {
        $phar = $this->args['f']['val'];
        $hash = $this->args['h']['val'];

        $phar->setSignatureAlgorithm($hash);
    }
    // }}}
    // {{{ public function cli_cmd_inf_meta_set
    /**
     * Cli Command Inf Meta Set
     *
     * @return string A description 
     */
    public function cli_cmd_inf_meta_set()
    {
        return "Set meta data of a PHAR entry or a PHAR package using serialized input. " . 
               "If no input file is specified for meta data then stdin is being used." . 
               "You can also specify a particular index using -k. In that case the metadata is " .
               "expected to be an array and the value of the given index is being set. If " .
               "the metadata is not present or empty a new array will be created. If the " .
               "metadata is present and a flat value then the return value is 1. Also using -k " .
               "the input is been taken directly rather then being serialized.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_meta_set
    /**
     * Cli Command Argument Meta Set
     *
     * @return array  The arguments for meta set
     */
    public function cli_cmd_arg_meta_set()
    {
        return self::phar_args('FekM', 'phar');
    }
    // }}}
    // {{{ public function cli_cmd_run_met_set
    /**
     * Cli Command Run Metaset
     *
     * @see $phar->startBuffering
     * @see $phar->setMetadata
     * @see $phar->stopBuffering
     */
    public function cli_cmd_run_meta_set()
    {
        $phar  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];
        $index = $this->args['k']['val'];
        $meta  = $this->args['m']['val'];

        $phar->startBuffering();

        if (isset($index)) {
            if (isset($entry)) {
                if ($phar[$entry]->hasMetadata()) {
                    $old = $phar[$entry]->getMetadata();
                } else {
                    $old = array();
                }
            } else {
                if ($phar->hasMetadata()) {
                    $old = $phar->getMetadata();
                } else {
                    $old = array();
                }
            }

            if (!is_array($old)) {
                self::error('Metadata is a flat value while an index operation was issued.');
            }

            $old[$index] = $meta;
            $meta = $old;
        } else {
            $meta = unserialize($meta);
        }
        
        if (isset($entry)) {
            $phar[$entry]->setMetadata($meta);
        } else {
            $phar->setMetadata($meta);
        }
        $phar->stopBuffering();
    }
    // }}}
    // {{{ public function cli_cmd_inf_met_get
    /**
     * Cli Command Inf Metaget
     *
     * @return string A description of the metaget arguments
     */
    public function cli_cmd_inf_meta_get()
    {
        return "Get meta information of a PHAR entry or a PHAR package in serialized from. " .
               "If no output file is specified for meta data then stdout is being used.\n" . 
               "You can also specify a particular index using -k. In that case the metadata is " .
               "expected to be an array and the value of the given index is returned using echo " .
               "rather than using serialize. If that index does not exist or no meta data is " .
               "present then the return value is 1.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_meta_get
    /**
     * Cli Command arg metaget
     *
     * @return array  The arguments for meta get.
     */
    public function cli_cmd_arg_meta_get()
    {
        return self::phar_args('Fek', 'phar');
    }
    // }}}
    // {{{ public function cli_cmd_run_meta_get
    /**
     * Cli Command Run Metaget
     *
     * @see $this->args
     * @see $phar[$x]->hasMetadata()
     * @see $phar->getMetadata()
     */
    public function cli_cmd_run_meta_get()
    {
        $phar  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];
        $index = $this->args['k']['val'];

        if (isset($entry)) {
            if (!$phar[$entry]->hasMetadata()) {
                exit(1);
            }
            echo serialize($phar[$entry]->getMetadata());
        } else {
            if (!$phar->hasMetadata()) {
                exit(1);
            }
            $meta = $phar->getMetadata();
        }

        if (isset($index)) {
            if (isset($index)) {
                if (isset($meta[$index])) {
                    echo $meta[$index];
                    exit(0);
                } else {
                    exit(1);
                }
            } else {
                echo serialize($meta);
            }
        }
    }
    // }}}
    // {{{ public function cli_cmd_inf_meta_del
    /**
     * Cli Command Inf Metadel
     *
     * @return string A description of the metadel function
     */
    public function cli_cmd_inf_meta_del()
    {
        return "Delete meta information of a PHAR entry or a PHAR package.\n" . 
               "If -k is given then the metadata is expected to be an array " . 
               "and the given index is being deleted.\n" .
               "If something was deleted the return value is 0 otherwise it is 1.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_meta_del
    /**
     * CliC ommand Arg Metadelete
     *
     * @return array The arguments for metadel
     */
    public function cli_cmd_arg_meta_del()
    {
        return self::phar_args('Fek', 'phar');
    }
    // }}}
    // {{{ public function cli_cmd_run_meta_del
    /**
     * Cli Command Run MetaDel
     *
     * @see $phar[$x]->delMetadata()
     * @see $phar->delMetadata()
     */
    public function cli_cmd_run_meta_del()
    {
        $phar  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];
        $index = $this->args['k']['val'];

        if (isset($entry)) {
            if (isset($index)) {
                if (!$phar[$entry]->hasMetadata()) {
                    exit(1);
                }
                $meta = $phar[$entry]->getMetadata();

                // @todo add error message here.
                if (!is_array($meta)) {
                    exit(1);
                }

                unset($meta[$index]);
                $phar[$entry]->setMetadata($meta);
            } else {
                exit($phar[$entry]->delMetadata() ? 0 : 1);
            }
        } else {
            if (isset($index)) {
                if (!$phar->hasMetadata()) {
                    exit(1);
                }

                $meta = $phar->getMetadata();

                // @todo Add error message
                if (!is_array($meta)) {
                    exit(1);
                }

                unset($meta[$index]);
                $phar->setMetadata($meta);
            } else {
                exit($phar->delMetadata() ? 0 : 1);
            }
        }
    }
    // }}}
    // {{{ public function cli_cmd_inf_info
    /**
     * CLi Command Inf Info
     *
     * @return string A description about the info commands.
     */
    public function cli_cmd_inf_info()
    {
        return "Get information about a PHAR package.\n" . 
               "By using -k it is possible to return a single value.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_info
    /**
     * Cli Command Arg Infos
     *
     * @return array The arguments for info command.
     */
    public function cli_cmd_arg_info()
    {
        return self::phar_args('Fk', 'phar');
    }
    // }}}
    // {{{ public function cli_cmd_run_info
    /**
     * Cli Command Run Info
     *
     * @param args $args
     */
    public function cli_cmd_run_info()
    {
        $phar  = $this->args['f']['val'];
        $index = $this->args['k']['val'];

        $hash  = $phar->getSignature();
        $infos = array();

        if ($phar->getAlias()) {
            $infos['Alias'] = $phar->getAlias();
        }

        if (!$hash) {
            $infos['Hash-type'] = 'NONE';
        } else {
            $infos['Hash-type'] = $hash['hash_type'];
            $infos['Hash'] = $hash['hash'];
        }

        $csize   = 0;
        $usize   = 0;
        $count   = 0;
        $ccount  = 0;
        $ucount  = 0;
        $mcount  = 0;
        $compalg = array('GZ'=>0, 'BZ2'=>0);

        foreach(new RecursiveIteratorIterator($phar) as $ent) {
            $count++;
            if ($ent->isCompressed()) {
                $ccount++;
                $csize += $ent->getCompressedSize();
                if ($ent->isCompressedGZ()) {
                    $compalg['GZ']++;
                } elseif ($ent->isCompressedBZIP2()) {
                    $compalg['BZ2']++;
                }
            } else {
                $ucount++;
                $csize += $ent->getSize();
            }
            
            $usize += $ent->getSize();
            
            if ($ent->hasMetadata()) {
                $mcount++;
            }
        }

        $infos['Entries']            = $count;
        $infos['Uncompressed-files'] = $ucount;
        $infos['Compressed-files']   = $ccount;
        $infos['Compressed-gz']      = $compalg['GZ'];
        $infos['Compressed-bz2']     = $compalg['BZ2'];
        $infos['Uncompressed-size']  = $usize;
        $infos['Compressed-size']    = $csize;
        $infos['Compression-ratio']  = sprintf('%.3g%%', $usize ? ($csize * 100) / $usize : 100);
        $infos['Metadata-global']    = $phar->hasMetadata() * 1;
        $infos['Metadata-files']     = $mcount;
        $infos['Stub-size']          = strlen($phar->getStub());

        if (isset($index)) {
            if (!isset($infos[$index])) {
                self::error("Requested value does not exist.\n");
            }

            echo $infos[$index];
            exit(0);
        }
        
        $l = 0;
        foreach($infos as $which => $val) {
            $l = max(strlen($which), $l);
        }
        
        foreach($infos as $which => $val) {
            echo $which . ':' . str_repeat(' ', $l + 1 - strlen($which)) . $val . "\n";
        }
    }
    // }}}
}
// }}}

}


new PharCommand($argc, $argv);
