<?php
//
// +----------------------------------------------------------------------+
// | PHP Version 4                                                        |
// +----------------------------------------------------------------------+
// | Copyright (c) 1997-2002 The PHP Group                                |
// +----------------------------------------------------------------------+
// | This source file is subject to version 2.02 of the PHP license,      |
// | that is bundled with this package in the file LICENSE, and is        |
// | available at through the world-wide-web at                           |
// | http://www.php.net/license/2_02.txt.                                 |
// | If you did not receive a copy of the PHP license and are unable to   |
// | obtain it through the world-wide-web, please send a note to          |
// | license@php.net so we can mail you a copy immediately.               |
// +----------------------------------------------------------------------+
// | Authors: Stig Bakken <ssb@fast.no>                                   |
// |          Tomas V.V.Cox <cox@idecnet.com>                             |
// +----------------------------------------------------------------------+
//
// $Id$

require_once 'PEAR.php';
require_once 'Archive/Tar.php';
require_once 'System.php';

// {{{ globals

/**
 * List of temporary files and directories registered by
 * PEAR_Common::addTempFile().
 * @var array
 */
$GLOBALS['_PEAR_Common_tempfiles'] = array();

/**
 * Valid maintainer roles
 * @var array
 */
$GLOBALS['_PEAR_Common_maintainer_roles'] = array('lead','developer','contributor','helper');

/**
 * Valid release states
 * @var array
 */
$GLOBALS['_PEAR_Common_release_states'] = array('alpha','beta','stable','snapshot','devel');

/**
 * Valid dependency types
 * @var array
 */
$GLOBALS['_PEAR_Common_dependency_types'] = array('pkg','ext','php','prog','ldlib','rtlib','os','websrv','sapi');

/**
 * Valid dependency relations
 * @var array
 */
$GLOBALS['_PEAR_Common_dependency_relations'] = array('has','eq','lt','le','gt','ge');

/**
 * Valid file roles
 * @var array
 */
$GLOBALS['_PEAR_Common_file_roles'] = array('php','ext','test','doc','data','extsrc','script');

/**
 * Valid replacement types
 * @var array
 */
$GLOBALS['_PEAR_Common_replacement_types'] = array('php-const', 'pear-config');

// }}}

/**
 * Class providing common functionality for PEAR adminsitration classes.
 */
class PEAR_Common extends PEAR
{
    // {{{ properties

    /** stack of elements, gives some sort of XML context */
    var $element_stack = array();

    /** name of currently parsed XML element */
    var $current_element;

    /** array of attributes of the currently parsed XML element */
    var $current_attributes = array();

    /** assoc with information about a package */
    var $pkginfo = array();

    /**
     * User Interface object (PEAR_Frontend_* class).  If null,
     * the log() method uses print.
     * @var object
     */
    var $ui = null;

    var $current_path = null;

    // }}}

    // {{{ constructor

    /**
     * PEAR_Common constructor
     *
     * @access public
     */
    function PEAR_Common()
    {
        $this->PEAR();
    }

    // }}}
    // {{{ destructor

    /**
     * PEAR_Common destructor
     *
     * @access private
     */
    function _PEAR_Common()
    {
        // doesn't work due to bug #14744
        //$tempfiles = $this->_tempfiles;
        $tempfiles =& $GLOBALS['_PEAR_Common_tempfiles'];
        while ($file = array_shift($tempfiles)) {
            if (@is_dir($file)) {
                System::rm("-rf $file");
            } elseif (file_exists($file)) {
                unlink($file);
            }
        }
    }

    // }}}
    // {{{ addTempFile()

    /**
     * Register a temporary file or directory.  When the destructor is
     * executed, all registered temporary files and directories are
     * removed.
     *
     * @param string  $file  name of file or directory
     *
     * @return void
     *
     * @access public
     */
    function addTempFile($file)
    {
        $GLOBALS['_PEAR_Common_tempfiles'][] = $file;
    }

    // }}}
    // {{{ mkDirHier()

    /**
     * Wrapper to System::mkDir(), creates a directory as well as
     * any necessary parent directories.
     *
     * @param string  $dir  directory name
     *
     * @return bool TRUE on success, or a PEAR error
     *
     * @access public
     */
    function mkDirHier($dir)
    {
        $this->log(2, "+ create dir $dir");
        return System::mkDir("-p $dir");
    }

    // }}}
    // {{{ log()

    /**
     * Logging method.
     *
     * @param int    $level  log level (0 is quiet, higher is noisier)
     * @param string $msg    message to write to the log
     *
     * @return void
     *
     * @access public
     */
    function log($level, $msg)
    {
        if ($this->debug >= $level) {
            if (is_object($this->ui)) {
                $this->ui->displayLine($msg);
            } else {
                print "$msg\n";
            }
        }
    }

    // }}}
    // {{{ mkTempDir()

    /**
     * Create and register a temporary directory.
     *
     * @param string $tmpdir (optional) Directory to use as tmpdir.
     *                       Will use system defaults (for example
     *                       /tmp or c:\windows\temp) if not specified
     *
     * @return string name of created directory
     *
     * @access public
     */
    function mkTempDir($tmpdir = '')
    {
        if ($tmpdir) {
            $topt = "-t $tmpdir ";
        } else {
            $topt = '';
        }
        if (!$tmpdir = System::mktemp($topt . '-d pear')) {
            return false;
        }
        $this->addTempFile($tmpdir);
        return $tmpdir;
    }

    // }}}
    // {{{ setFrontend()

    function setFrontend(&$ui)
    {
        $this->ui = &$ui;
    }

    // }}}

    // {{{ _element_start()

    /**
     * XML parser callback for starting elements.  Used while package
     * format version is not yet known.
     *
     * @param resource  $xp       XML parser resource
     * @param string    $name     name of starting element
     * @param array     $attribs  element attributes, name => value
     *
     * @return void
     *
     * @access private
     */
    function _element_start($xp, $name, $attribs)
    {
        array_push($this->element_stack, $name);
        $this->current_element = $name;
        $spos = sizeof($this->element_stack) - 2;
        $this->prev_element = ($spos >= 0) ? $this->element_stack[$spos] : '';
        $this->current_attributes = $attribs;
        switch ($name) {
            case 'package': {
                if (isset($attribs['version'])) {
                    $vs = preg_replace('/[^0-9a-z]/', '_', $attribs['version']);
                } else {
                    $vs = '1_0';
                }
                $elem_start = '_element_start_'. $vs;
                $elem_end = '_element_end_'. $vs;
                $cdata = '_pkginfo_cdata_'. $vs;
                xml_set_element_handler($xp, $elem_start, $elem_end);
                xml_set_character_data_handler($xp, $cdata);
                break;
            }
        }
    }

    // }}}
    // {{{ _element_end()

    /**
     * XML parser callback for ending elements.  Used while package
     * format version is not yet known.
     *
     * @param resource  $xp    XML parser resource
     * @param string    $name  name of ending element
     *
     * @return void
     *
     * @access private
     */
    function _element_end($xp, $name)
    {
    }

    // }}}

    // Support for package DTD v1.0:
    // {{{ _element_start_1_0()

    /**
     * XML parser callback for ending elements.  Used for version 1.0
     * packages.
     *
     * @param resource  $xp    XML parser resource
     * @param string    $name  name of ending element
     *
     * @return void
     *
     * @access private
     */
    function _element_start_1_0($xp, $name, $attribs)
    {
        array_push($this->element_stack, $name);
        $this->current_element = $name;
        $spos = sizeof($this->element_stack) - 2;
        $this->prev_element = ($spos >= 0) ? $this->element_stack[$spos] : '';
        $this->current_attributes = $attribs;
        $this->cdata = '';
        switch ($name) {
            case 'dir':
                if ($this->in_changelog) {
                    break;
                }
                if ($attribs['name'] != '/') {
                    $this->dir_names[] = $attribs['name'];
                }
                if (isset($attribs['baseinstalldir'])) {
                    $this->dir_install = $attribs['baseinstalldir'];
                }
                if (isset($attribs['role'])) {
                    $this->dir_role = $attribs['role'];
                }
                break;
            case 'file':
                if ($this->in_changelog) {
                    break;
                }
                if (isset($attribs['name'])) {
                    $path = '';
                    if (count($this->dir_names)) {
                        foreach ($this->dir_names as $dir) {
                            $path .= $dir . DIRECTORY_SEPARATOR;
                        }
                    }
                    $path .= $attribs['name'];
                    unset($attribs['name']);
                    $this->current_path = $path;
                    $this->filelist[$path] = $attribs;
                    // Set the baseinstalldir only if the file don't have this attrib
                    if (!isset($this->filelist[$path]['baseinstalldir']) &&
                        isset($this->dir_install))
                    {
                        $this->filelist[$path]['baseinstalldir'] = $this->dir_install;
                    }
                    // Set the Role
                    if (!isset($this->filelist[$path]['role']) && isset($this->dir_role)) {
                        $this->filelist[$path]['role'] = $this->dir_role;
                    }
                }
                break;
            case 'replace':
                if (!$this->in_changelog) {
                    $this->filelist[$this->current_path]['replacements'][] = $attribs;
                }
                break;

            case 'libfile':
                if (!$this->in_changelog) {
                    $this->lib_atts = $attribs;
                    $this->lib_atts['role'] = 'extsrc';
                }
                break;
            case 'maintainers':
                $this->pkginfo['maintainers'] = array();
                $this->m_i = 0; // maintainers array index
                break;
            case 'maintainer':
                // compatibility check
                if (!isset($this->pkginfo['maintainers'])) {
                    $this->pkginfo['maintainers'] = array();
                    $this->m_i = 0;
                }
                $this->pkginfo['maintainers'][$this->m_i] = array();
                $this->current_maintainer =& $this->pkginfo['maintainers'][$this->m_i];
                break;
            case 'changelog':
                $this->pkginfo['changelog'] = array();
                $this->c_i = 0; // changelog array index
                $this->in_changelog = true;
                break;
            case 'release':
                if ($this->in_changelog) {
                    $this->pkginfo['changelog'][$this->c_i] = array();
                    $this->current_release = &$this->pkginfo['changelog'][$this->c_i];
                } else {
                    $this->current_release = &$this->pkginfo;
                }
                break;
            case 'deps':
                if (!$this->in_changelog) {
                    $this->pkginfo['release_deps'] = array();
                }
                break;
            case 'dep':
                // dependencies array index
                if (!$this->in_changelog) {
                    $this->d_i = (isset($this->d_i)) ? $this->d_i + 1 : 0;
                    $this->pkginfo['release_deps'][$this->d_i] = $attribs;
                }
                break;
        }
    }

    // }}}
    // {{{ _element_end_1_0()

    /**
     * XML parser callback for ending elements.  Used for version 1.0
     * packages.
     *
     * @param resource  $xp    XML parser resource
     * @param string    $name  name of ending element
     *
     * @return void
     *
     * @access private
     */
    function _element_end_1_0($xp, $name)
    {
        $data = trim($this->cdata);
        switch ($name) {
            case 'name':
                switch ($this->prev_element) {
                    case 'package':
                        $this->pkginfo['package'] = ereg_replace('[^a-zA-Z0-9._]', '_', $data);
                        break;
                    case 'maintainer':
                        $this->current_maintainer['name'] = $data;
                        break;
                }
                break;
            case 'summary':
                $this->pkginfo['summary'] = $data;
                break;
            case 'description':
                $this->pkginfo['description'] = $data;
                break;
            case 'user':
                $this->current_maintainer['handle'] = $data;
                break;
            case 'email':
                $this->current_maintainer['email'] = $data;
                break;
            case 'role':
                $this->current_maintainer['role'] = $data;
                break;
            case 'version':
                $data = ereg_replace ('[^a-zA-Z0-9._\-]', '_', $data);
                if ($this->in_changelog) {
                    $this->current_release['version'] = $data;
                } else {
                    $this->pkginfo['version'] = $data;
                }
                break;
            case 'date':
                if ($this->in_changelog) {
                    $this->current_release['release_date'] = $data;
                } else {
                    $this->pkginfo['release_date'] = $data;
                }
                break;
            case 'notes':
                if ($this->in_changelog) {
                    $this->current_release['release_notes'] = $data;
                } else {
                    $this->pkginfo['release_notes'] = $data;
                }
                break;
            case 'state':
                if ($this->in_changelog) {
                    $this->current_release['release_state'] = $data;
                } else {
                    $this->pkginfo['release_state'] = $data;
                }
                break;
            case 'license':
                $this->pkginfo['release_license'] = $data;
                break;
            case 'sources':
                $this->lib_sources[] = $data;
                break;
            case 'dep':
                if ($data = trim($data)) {
                    $this->pkginfo['release_deps'][$this->d_i]['name'] = $data;
                }
                break;
            case 'dir':
                if ($this->in_changelog) {
                    break;
                }
                array_pop($this->dir_names);
                break;
            case 'file':
                if ($this->in_changelog) {
                    break;
                }
                if ($data) {
                    $path = '';
                    if (count($this->dir_names)) {
                        foreach ($this->dir_names as $dir) {
                            $path .= $dir . DIRECTORY_SEPARATOR;
                        }
                    }
                    $path .= $data;
                    $this->filelist[$path] = $this->current_attributes;
                    // Set the baseinstalldir only if the file don't have this attrib
                    if (!isset($this->filelist[$path]['baseinstalldir']) &&
                        isset($this->dir_install))
                    {
                        $this->filelist[$path]['baseinstalldir'] = $this->dir_install;
                    }
                    // Set the Role
                    if (!isset($this->filelist[$path]['role']) && isset($this->dir_role)) {
                        $this->filelist[$path]['role'] = $this->dir_role;
                    }
                }
                break;
            case 'libfile':
                if ($this->in_changelog) {
                    break;
                }
                $path = '';
                if (!empty($this->dir_names)) {
                    foreach ($this->dir_names as $dir) {
                        $path .= $dir . DIRECTORY_SEPARATOR;
                    }
                }
                $path .= $this->lib_name;
                $this->filelist[$path] = $this->lib_atts;
                // Set the baseinstalldir only if the file don't have this attrib
                if (!isset($this->filelist[$path]['baseinstalldir']) &&
                    isset($this->dir_install))
                {
                    $this->filelist[$path]['baseinstalldir'] = $this->dir_install;
                }
                if (isset($this->lib_sources)) {
                    $this->filelist[$path]['sources'] = implode(' ', $this->lib_sources);
                }
                unset($this->lib_atts);
                unset($this->lib_sources);
                unset($this->lib_name);
                break;
            case 'libname':
                if ($this->in_changelog) {
                    break;
                }
                $this->lib_name = $data;
                break;
            case 'maintainer':
                if (empty($this->pkginfo['maintainers'][$this->m_i]['role'])) {
                    $this->pkginfo['maintainers'][$this->m_i]['role'] = 'lead';
                }
                $this->m_i++;
                break;
            case 'release':
                if ($this->in_changelog) {
                    $this->c_i++;
                }
                break;
            case 'changelog':
                $this->in_changelog = false;
                break;
            case 'summary':
                $this->pkginfo['summary'] = $data;
                break;
        }
        array_pop($this->element_stack);
        $spos = sizeof($this->element_stack) - 1;
        $this->current_element = ($spos > 0) ? $this->element_stack[$spos] : '';
    }

    // }}}
    // {{{ _pkginfo_cdata_1_0()

    /**
     * XML parser callback for character data.  Used for version 1.0
     * packages.
     *
     * @param resource  $xp    XML parser resource
     * @param string    $name  character data
     *
     * @return void
     *
     * @access private
     */
    function _pkginfo_cdata_1_0($xp, $data)
    {
        if (isset($this->cdata)) {
            $this->cdata .= $data;
        }
    }

    // }}}

    // {{{ infoFromTgzFile()

    /**
     * Returns information about a package file.  Expects the name of
     * a gzipped tar file as input.
     *
     * @param string  $file  name of .tgz file
     *
     * @return array  array with package information
     *
     * @access public
     *
     */
    function infoFromTgzFile($file)
    {
        if (!@is_file($file)) {
            return $this->raiseError('tgz :: could not open file');
        }
        if (substr($file, -4) == '.tar') {
            $compress = false;
        } else {
            $compress = true;
        }
        $tar = new Archive_Tar($file, $compress);
        $content = $tar->listContent();
        if (!is_array($content)) {
            return $this->raiseError('tgz :: could not get contents of package');
        }
        $xml = null;
        foreach ($content as $file) {
            $name = $file['filename'];
            if ($name == 'package.xml') {
                $xml = $name;
            } elseif (ereg('^.*/package.xml$', $name, $match)) {
                $xml = $match[0];
            }
        }
        $tmpdir = System::mkTemp('-d pear');
        $this->addTempFile($tmpdir);
        if (!$xml || !$tar->extractList($xml, $tmpdir)) {
            return $this->raiseError('tgz :: could not extract the package.xml file');
        }
        return $this->infoFromDescriptionFile("$tmpdir/$xml");
    }

    // }}}
    // {{{ infoFromDescriptionFile()

    /**
     * Returns information about a package file.  Expects the name of
     * a package xml file as input.
     *
     * @param string  $descfile  name of package xml file
     *
     * @return array  array with package information
     *
     * @access public
     *
     */
    function infoFromDescriptionFile($descfile)
    {
        if (!@is_file($descfile) || !is_readable($descfile) ||
             (!$fp = @fopen($descfile, 'r'))) {
            return $this->raiseError("Unable to open $descfile");
        }

        // read the whole thing so we only get one cdata callback
        // for each block of cdata
        $data = fread($fp, filesize($descfile));
        return $this->infoFromString($data);
    }

    // }}}
    // {{{ infoFromString()

    /**
     * Returns information about a package file.  Expects the contents
     * of a package xml file as input.
     *
     * @param string  $data  name of package xml file
     *
     * @return array   array with package information
     *
     * @access public
     *
     */
    function infoFromString($data)
    {
        $xp = @xml_parser_create();
        if (!$xp) {
            return $this->raiseError('Unable to create XML parser');
        }
        xml_set_object($xp, $this);
        xml_set_element_handler($xp, '_element_start', '_element_end');
        xml_set_character_data_handler($xp, '_pkginfo_cdata');
        xml_parser_set_option($xp, XML_OPTION_CASE_FOLDING, false);

        $this->element_stack = array();
        $this->pkginfo = array();
        $this->current_element = false;
        $this->destdir = '';
        $this->pkginfo['filelist'] = array();
        $this->filelist =& $this->pkginfo['filelist'];
        $this->dir_names = array();
        $this->in_changelog = false;

        if (!xml_parse($xp, $data, 1)) {
            $code = xml_get_error_code($xp);
            $msg = sprintf("XML error: %s at line %d",
                           xml_error_string($code),
                           xml_get_current_line_number($xp));
            xml_parser_free($xp);
            return $this->raiseError($msg, $code);
        }

        xml_parser_free($xp);

        foreach ($this->pkginfo as $k => $v) {
            if (!is_array($v)) {
                $this->pkginfo[$k] = trim($v);
            }
        }
        return $this->pkginfo;
    }
    // }}}
    // {{{ xmlFromInfo()

    /**
     * Return an XML document based on the package info (as returned
     * by the PEAR_Common::infoFrom* methods).
     *
     * @param array  $pkginfo  package info
     *
     * @return string XML data
     *
     * @access public
     */
    function xmlFromInfo($pkginfo)
    {
        static $maint_map = array(
            "handle" => "user",
            "name" => "name",
            "email" => "email",
            "role" => "role",
            );
        $ret = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
        //$ret .= "<!DOCTYPE package SYSTEM \"http://pear.php.net/package10.dtd\">\n";
        $ret .= "<package version=\"1.0\">
  <name>$pkginfo[package]</name>
  <summary>".htmlspecialchars($pkginfo['summary'])."</summary>
  <description>".htmlspecialchars($pkginfo['description'])."</description>
  <maintainers>
";
        foreach ($pkginfo['maintainers'] as $maint) {
            $ret .= "    <maintainer>\n";
            foreach ($maint_map as $idx => $elm) {
                $ret .= "      <$elm>";
                $ret .= htmlspecialchars($maint[$idx]);
                $ret .= "</$elm>\n";
            }
            $ret .= "    </maintainer>\n";
        }
        $ret .= "  </maintainers>\n";
        $ret .= $this->_makeReleaseXml($pkginfo);
        if (@sizeof($pkginfo['changelog']) > 0) {
            $ret .= "  <changelog>\n";
            foreach ($pkginfo['changelog'] as $oldrelease) {
                $ret .= $this->_makeReleaseXml($oldrelease, true);
            }
            $ret .= "  </changelog>\n";
        }
        $ret .= "</package>\n";
        return $ret;
    }

    // }}}
    // {{{ _makeReleaseXml()

    /**
     * Generate part of an XML description with release information.
     *
     * @param array  $pkginfo    array with release information
     * @param bool   $changelog  whether the result will be in a changelog element
     *
     * @return string XML data
     *
     * @access private
     */
    function _makeReleaseXml($pkginfo, $changelog = false)
    {
        // XXX QUOTE ENTITIES IN PCDATA, OR EMBED IN CDATA BLOCKS!!
        $indent = $changelog ? "  " : "";
        $ret = "$indent  <release>\n";
        if (!empty($pkginfo['version'])) {
            $ret .= "$indent    <version>$pkginfo[version]</version>\n";
        }
        if (!empty($pkginfo['release_date'])) {
            $ret .= "$indent    <date>$pkginfo[release_date]</date>\n";
        }
        if (!empty($pkginfo['release_license'])) {
            $ret .= "$indent    <license>$pkginfo[release_license]</license>\n";
        }
        if (!empty($pkginfo['release_state'])) {
            $ret .= "$indent    <state>$pkginfo[release_state]</state>\n";
        }
        if (!empty($pkginfo['release_notes'])) {
            $ret .= "$indent    <notes>".htmlspecialchars($pkginfo['release_notes'])."</notes>\n";
        }
        if (isset($pkginfo['release_deps']) && sizeof($pkginfo['release_deps']) > 0) {
            $ret .= "$indent    <deps>\n";
            foreach ($pkginfo['release_deps'] as $dep) {
                $ret .= "$indent      <dep type=\"$dep[type]\" rel=\"$dep[rel]\"";
                if (isset($dep['version'])) {
                    $ret .= " version=\"$dep[version]\"";
                }
                if (isset($dep['name'])) {
                    $ret .= ">$dep[name]</dep>\n";
                } else {
                    $ret .= "/>\n";
                }
            }
            $ret .= "$indent    </deps>\n";
        }
        if (isset($pkginfo['filelist'])) {
            $ret .= "$indent    <filelist>\n";
            foreach ($pkginfo['filelist'] as $file => $fa) {
                if (@$fa['role'] == 'extsrc') {
                    $ret .= "$indent      <libfile>\n";
                    $ret .= "$indent        <libname>$file</libname>\n";
                    $ret .= "$indent        <sources>$fa[sources]</sources>\n";
                    $ret .= "$indent      </libfile>\n";
                } else {
                    @$ret .= "$indent      <file role=\"$fa[role]\"";
                    if (isset($fa['baseinstalldir'])) {
                        $ret .= ' baseinstalldir="' .
                             htmlspecialchars($fa['baseinstalldir']) . '"';
                    }
                    if (isset($fa['md5sum'])) {
                        $ret .= " md5sum=\"$fa[md5sum]\"";
                    }
                    if (!empty($fa['install-as'])) {
                        $ret .= ' install-as="' .
                             htmlspecialchars($fa['install-as']) . '"';
                    }
                    $ret .= ' name="' . htmlspecialchars($file) . '"';
                    if (empty($fa['replacements'])) {
                        $ret .= "/>\n";
                    } else {
                        $ret .= ">\n";
                        foreach ($fa['replacements'] as $r) {
                            $ret .= "$indent        <replace";
                            foreach ($r as $k => $v) {
                                $ret .= " $k=\"" . htmlspecialchars($v) .'"';
                            }
                            $ret .= "/>\n";
                        }
                        @$ret .= "$indent      </file>\n";
                    }
                }
            }
            $ret .= "$indent    </filelist>\n";
        }
        $ret .= "$indent  </release>\n";
        return $ret;
    }

    // }}}
    // {{{ _infoFromAny()

    function _infoFromAny($info)
    {
        if (is_string($info) && file_exists($info)) {
            $tmp = substr($info, -4);
            if ($tmp == '.xml') {
                $info = $this->infoFromDescriptionFile($info);
            } elseif ($tmp == '.tar' || $tmp == '.tgz') {
                $info = $this->infoFromTgzFile($info);
            } else {
                $fp = fopen($params[0], "r");
                $test = fread($fp, 5);
                fclose($fp);
                if ($test == "<?xml") {
                    $info = $obj->infoFromDescriptionFile($info);
                } else {
                    $info = $obj->infoFromTgzFile($info);
                }
            }
            if (PEAR::isError($info)) {
                return $this->raiseError($info);
            }
        }
        return $info;
    }

    // }}}
    // {{{ validatePackageInfo()

    function validatePackageInfo($info, &$errors, &$warnings)
    {
        global $_PEAR_Common_maintainer_roles,
            $_PEAR_Common_release_states,
            $_PEAR_Common_dependency_types,
            $_PEAR_Common_dependency_relations,
            $_PEAR_Common_file_roles,
            $_PEAR_Common_replacement_types;
        if (PEAR::isError($info = $this->_infoFromAny($info))) {
            return $this->raiseError($info);
        }
        if (!is_array($info)) {
            return false;
        }
        $errors = array();
        $warnings = array();
        if (empty($info['package'])) {
            $errors[] = 'missing package name';
        }
        if (empty($info['summary'])) {
            $errors[] = 'missing summary';
        } elseif (strpos(trim($info['summary']), "\n") !== false) {
            $warnings[] = 'summary should be on a single line';
        }
        if (empty($info['description'])) {
            $errors[] = 'missing description';
        }
        if (empty($info['release_license'])) {
            $errors[] = 'missing license';
        }
        if (empty($info['version'])) {
            $errors[] = 'missing version';
        }
        if (empty($info['release_state'])) {
            $errors[] = 'missing release state';
        } elseif (!in_array($info['release_state'], $_PEAR_Common_release_states)) {
            $errors[] = "invalid release state `$info[release_state]', should be one of: ".implode(' ', $_PEAR_Common_release_states);
        }
        if (empty($info['release_date'])) {
            $errors[] = 'missing release date';
        } elseif (!preg_match('/^\d{4}-\d\d-\d\d$/', $info['release_date'])) {
            $errors[] = "invalid release date `$info[release_date]', format is YYYY-MM-DD";
        }
        if (empty($info['release_notes'])) {
            $errors[] = "missing release notes";
        }
        if (empty($info['maintainers'])) {
            $errors[] = 'no maintainer(s)';
        } else {
            $i = 1;
            foreach ($info['maintainers'] as $m) {
                if (empty($m['handle'])) {
                    $errors[] = "maintainer $i: missing handle";
                }
                if (empty($m['role'])) {
                    $errors[] = "maintainer $i: missing role";
                } elseif (!in_array($m['role'], $_PEAR_Common_maintainer_roles)) {
                    $errors[] = "maintainer $i: invalid role `$m[role]', should be one of: ".implode(' ', $_PEAR_Common_maintainer_roles);
                }
                if (empty($m['name'])) {
                    $errors[] = "maintainer $i: missing name";
                }
                if (empty($m['email'])) {
                    $errors[] = "maintainer $i: missing email";
                }
                $i++;
            }
        }
        if (!empty($info['deps'])) {
            $i = 1;
            foreach ($info['deps'] as $d) {
                if (empty($d['type'])) {
                    $errors[] = "depenency $i: missing type";
                } elseif (!in_array($d['type'], $_PEAR_Common_dependency_types)) {
                    $errors[] = "dependency $i: invalid type, should be one of: ".implode(' ', $_PEAR_Common_depenency_types);
                }
                if (empty($d['rel'])) {
                    $errors[] = "dependency $i: missing relation";
                } elseif (!in_array($d['rel'], $_PEAR_Common_dependency_relations)) {
                    $errors[] = "dependency $i: invalid relation, should be one of: ".implode(' ', $_PEAR_Common_dependency_relations);
                }
                if ($d['rel'] != 'has' && empty($d['version'])) {
                    $warnings[] = "dependency $i: missing version";
                } elseif ($d['rel'] == 'has' && !empty($d['version'])) {
                    $warnings[] = "dependency $i: version ignored for `has' dependencies";
                }
                if ($d['type'] == 'php' && !empty($d['name'])) {
                    $warnings[] = "dependency $i: name ignored for php type dependencies";
                } elseif ($d['type'] != 'php' && empty($d['name'])) {
                    $errors[] = "dependency $i: missing name";
                }
                $i++;
            }
        }
        if (empty($info['filelist'])) {
            $errors[] = 'no files';
        } else {
            foreach ($info['filelist'] as $file => $fa) {
                if (empty($fa['role'])) {
                    $errors[] = "file $file: missing role";
                } elseif (!in_array($fa['role'], $_PEAR_Common_file_roles)) {
                    $errors[] = "file $file: invalid role, should be one of: ".implode(' ', $_PEAR_Common_file_roles);
                } elseif ($fa['role'] == 'extsrc' && empty($fa['sources'])) {
                    $errors[] = "file $file: no source files";
                }
                // (ssb) Any checks we can do for baseinstalldir?
                // (cox) Perhaps checks that either the target dir and baseInstall
                //       doesn't cointain "../../"
            }
        }
        return true;
    }

    // }}}
    // {{{ analyzeSourceCode()

    function analyzeSourceCode($file)
    {
        if (!function_exists("token_get_all")) {
            return false;
        }
        if (!$fp = @fopen($file, "r")) {
            return false;
        }
        $contents = fread($fp, filesize($file));
        $tokens = token_get_all($contents);
        for ($i = 0; $i < sizeof($tokens); $i++) {
            list($token, $data) = $tokens[$i];
            if (is_string($token)) {
                var_dump($token);
            } else {
                print token_name($token) . ' ';
                var_dump(rtrim($data));
            }
        }
        $look_for = 0;
        $paren_level = 0;
        $bracket_level = 0;
        $brace_level = 0;
        $lastphpdoc = '';
        $current_class = '';
        $current_class_level = -1;
        $current_function = '';
        $current_function_level = -1;
        $declared_classes = array();
        $declared_functions = array();
        $declared_methods = array();
        $used_classes = array();
        $used_functions = array();
        for ($i = 0; $i < sizeof($tokens); $i++) {
            list($token, $data) = $tokens[$i];
            switch ($token) {
                case '{':
                    $brace_level++;
                    continue 2;
                case '}':
                    $brace_level--;
                    if ($current_class_level == $brace_level) {
                        $current_class = '';
                        $current_class_level = -1;
                    }
                    if ($current_function_level == $brace_level) {
                        $current_function = '';
                        $current_function_level = -1;
                    }
                    continue 2;
                case '[': $bracket_level++; continue 2;
                case ']': $bracket_level--; continue 2;
                case '(': $paren_level++;   continue 2;
                case ')': $paren_level--;   continue 2;
                case T_CLASS:
                case T_FUNCTION:
                case T_NEW:
                    $look_for = $token;
                    continue 2;
                case T_STRING:
                    if ($look_for == T_CLASS) {
                        $current_class = $data;
                        $current_class_level = $brace_level;
                        $declared_classes[] = $current_class;
                    } elseif ($look_for == T_FUNCTION) {
                        if ($current_class) {
                            $current_function = "$current_class::$data";
                            $declared_methods[$current_class][] = $data;
                        } else {
                            $current_function = $data;
                        }
                        $current_function_level = $brace_level;
                        $declared_functions[] = $current_function;
                    } elseif ($look_for == T_NEW) {
                        $used_classes[$data] = true;
                    }
                    $look_for = 0;
                    continue 2;
                case T_COMMENT:
                    if (preg_match('!^/\*\*\s!', $data)) {
                        $lastphpdoc = $data;
                        //$j = $i;
                        //while ($tokens[$j][0] == T_WHITESPACE) $j++;
                        // the declaration that the phpdoc applies to
                        // is at $tokens[$j] now
                    }
                    continue 2;
                case T_DOUBLE_COLON:
                    $used_classes[$tokens[$i - 1][1]] = true;
                    continue 2;
            }
        }
        return array(
            "declared_classes" => $declared_classes,
            "declared_methods" => $declared_methods,
            "declared_functions" => $declared_functions,
            "used_classes" => array_keys($used_classes),
            );
    }

    // }}}
    // {{{ detectDependencies()

    function detectDependencies($any, $status_callback = null)
    {
        if (!function_exists("token_get_all")) {
            return false;
        }
        if (PEAR::isError($info = $this->_infoFromAny($any))) {
            return $this->raiseError($info);
        }
        if (!is_array($info)) {
            return false;
        }
        $deps = array();
        $used_c = $decl_c = array();
        foreach ($info['filelist'] as $file => $fa) {
            $tmp = $this->analyzeSourceCode($file);
            $used_c = @array_merge($used_c, $tmp['used_classes']);
            $decl_c = @array_merge($decl_c, $tmp['declared_classes']);
        }
        $used_c = array_unique($used_c);
        $decl_c = array_unique($decl_c);
        $undecl_c = array_diff($used_c, $decl_c);
        return array('used_classes' => $used_c,
                     'declared_classes' => $decl_c,
                     'undeclared_classes' => $undecl_c);
    }

    // }}}
    // {{{ getUserRoles()

    /**
     * Get the valid roles for a PEAR package maintainer
     *
     * @return array
     * @static
     */
    function getUserRoles()
    {
        return $GLOBALS['_PEAR_Common_maintainer_roles'];
    }

    // }}}
    // {{{ getReleaseStates()

    /**
     * Get the valid package release states of packages
     *
     * @return array
     * @static
     */
    function getReleaseStates()
    {
        return $GLOBALS['_PEAR_Common_release_states'];
    }

    // }}}
    // {{{ getDependencyTypes()

    /**
     * Get the implemented dependency types (php, ext, pkg etc.)
     *
     * @return array
     * @static
     */
    function getDependencyTypes()
    {
        return $GLOBALS['_PEAR_Common_dependency_types'];
    }

    // }}}
    // {{{ getDependencyRelations()

    /**
     * Get the implemented dependency relations (has, lt, ge etc.)
     *
     * @return array
     * @static
     */
    function getDependencyRelations()
    {
        return $GLOBALS['_PEAR_Common_dependency_relations'];
    }

    // }}}
    // {{{ getFileRoles()

    /**
     * Get the implemented file roles
     *
     * @return array
     * @static
     */
    function getFileRoles()
    {
        return $GLOBALS['_PEAR_Common_file_roles'];
    }

    // }}}
    // {{{ getReplacementTypes()

    /**
     * Get the implemented file replacement types in
     *
     * @return array
     * @static
     */
    function getReplacementTypes()
    {
        return $GLOBALS['_PEAR_Common_replacement_types'];
    }

    // }}}

    // {{{ downloadHttp()

    /**
     * Download a file through HTTP.  Considers suggested file name in
     * Content-disposition: header and can run a callback function for
     * different events.  The callback will be called with two
     * parameters: the callback type, and parameters.  The implemented
     * callback types are:
     *
     *  'message'   the parameter is a string with an informational message
     *  'saveas'    may be used to save with a different file name, the
     *              parameter is array($ui, $filaneme) where $ui is the
     *              user interface object used (instance of PEAR_Frontend_*)
     *              and $filename is the filename that is about to be used.
     *              If a 'saveas' callback returns a non-empty string,
     *              that file name will be used instead.  Note that
     *              $save_dir will not be affected by this, only the
     *              basename of the file. 
     *  'start'     download is starting, parameter is number of bytes
     *              that are expected, or -1 if unknown
     *  'bytesread' parameter is the number of bytes read so far
     *  'done'      download is complete, parameter is the total number
     *              of bytes read
     *
     * If an HTTP proxy has been configured (http_proxy PEAR_Config
     * setting), the proxy will be used.
     *
     * @param string  $url       the URL to download
     * @param object  $ui        PEAR_Frontend_* instance
     * @param object  $config    PEAR_Config instance
     * @param string  $save_dir  (optional) directory to save file in
     * @param mixed   $callback  (optional) function/method to call for status
     *                           updates
     *
     * @return string  Returns the full path of the downloaded file or a PEAR
     *                 error on failure.  If the error is caused by
     *                 socket-related errors, the error object will
     *                 have the fsockopen error code available through
     *                 getCode().
     *
     * @access public
     */
    function downloadHttp($url, &$ui, &$config, $save_dir = '.',
                          $callback = null)
    {
        if (preg_match('!^http://([^/:?#]*)(:(\d+))?(/.*)!', $url, $matches)) {
            list(,$host,,$port,$path) = $matches;
        }
        $proxy_host = $proxy_port = null;
        if ($proxy = $config->get('http_proxy')) {
            list($proxy_host, $proxy_port) = explode(':', $proxy);
            if (empty($proxy_port)) {
                $proxy_port = 8080;
            }
            if ($callback) {
                call_user_func($callback, 'message', "Using HTTP proxy $host:$port");
            }
        }
        if (empty($port)) {
            $port = 80;
        }
        if (!extension_loaded("zlib")) {
            $pkgfile .= '?uncompress=yes';
        }
        if ($proxy_host) {
            $fp = @fsockopen($proxy_host, $proxy_port, $errno, $errstr);
            if (!$fp) {
                return PEAR::raiseError("Connection to `$proxy_host:$proxy_port' failed: $errstr", $errno);
            }
            $request = "GET $url HTTP/1.0\r\n";
        } else {
            $fp = @fsockopen($host, $port, $errno, $errstr);
            if (!$fp) {
                return PEAR::raiseError("Connection to `$host:$port' failed: $errstr", $errno);
            }
            $request = "GET $path HTTP/1.0\r\n";
        }
        $request .= "Host: $host:$port\r\n".
            "User-Agent: ".PHP_VERSION."\r\n".
            "\r\n";
        fwrite($fp, $request);
        $headers = array();
        while (trim($line = fgets($fp, 1024))) {
            if (preg_match('/^([^:]+):\s+(.*)\s*$/', $line, $matches)) {
                $headers[strtolower($matches[1])] = trim($matches[2]);
            }
        }
        if (isset($headers['content-disposition']) &&
            preg_match('/\sfilename=\"([^;]*\S)\"\s*(;|$)/', $headers['content-disposition'], $matches)) {
            $save_as = basename($matches[1]);
        } else {
            $save_as = basename($url);
        }
        if ($callback) {
            $tmp = call_user_func($callback, 'saveas', array(&$ui, $save_as));
            if ($tmp) {
                $save_as = $tmp;
            }
        }
        $dest_file = $save_dir . DIRECTORY_SEPARATOR . $save_as;
        if (!$wp = @fopen($dest_file, 'wb')) {
            fclose($fp);
            return PEAR::raiseError("could not open $dest_file for writing");
        }
        if (isset($headers['content-length'])) {
            $length = $headers['content-length'];
        } else {
            $length = -1;
        }
        $bytes = 0;
        if ($callback) {
            call_user_func($callback, 'start', $length);
        }
        while ($data = @fread($fp, 1024)) {
            $bytes += strlen($data);
            if ($callback) {
                call_user_func($callback, 'bytesread', $bytes);
            }
            if (!@fwrite($wp, $data)) {
                fclose($fp);
                return PEAR::raiseError("$pkgfile: write failed ($php_errormsg)");
            }
        }
        fclose($fp);
        fclose($wp);
        if ($callback) {
            call_user_func($callback, 'done', $bytes);
        }
        // callback: done!
        return $dest_file;
    }

    // }}}
}

?>
