<?php
// /* vim: set expandtab tabstop=4 shiftwidth=4: */
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
// | Author: Stig Bakken <ssb@fast.no>                                    |
// |                                                                      |
// +----------------------------------------------------------------------+
//
// $Id$

require_once 'PEAR/Command/Common.php';
require_once 'PEAR/Common.php';
require_once 'PEAR/Remote.php';
require_once 'PEAR/Registry.php';

class PEAR_Command_Remote extends PEAR_Command_Common
{
    // {{{ command definitions

    var $commands = array(
        'remote-info' => array(
            'summary' => 'Information About Remote Packages',
            'function' => 'doRemoteInfo',
            'shortcut' => 'ri',
            'options' => array(),
            'doc' => '<package>
Get details on a package from the server.',
            ),
        'list-upgrades' => array(
            'summary' => 'List Available Upgrades',
            'function' => 'doListUpgrades',
            'shortcut' => 'lu',
            'options' => array(),
            'doc' => '
List releases on the server of packages you have installed where
a newer version is available with the same release state (stable etc.).'
            ),
        'remote-list' => array(
            'summary' => 'List Remote Packages',
            'function' => 'doRemoteList',
            'shortcut' => 'rl',
            'options' => array(),
            'doc' => '
Lists the packages available on the configured server along with the
latest stable release of each package.',
            ),
        'search' => array(
            'summary' => 'Search Packagesdatabase',
            'function' => 'doSearch',
            'shortcut' => 'sp',
            'options' => array(),
            'doc' => '
Lists all packages which match the search paramteres (first param 
is package name, second package info)',
            ),
        'list-all' => array(
            'summary' => 'List All Packages',
            'function' => 'doListAll',
            'shortcut' => 'la',
            'options' => array(),
            'doc' => '
Lists the packages available on the configured server along with the
latest stable release of each package.',
            ),
        'download' => array(
            'summary' => 'Download Package',
            'function' => 'doDownload',
            'shortcut' => 'd',
            'options' => array(
                'nocompress' => array(
                    'shortopt' => 'Z',
                    'doc' => 'download an uncompressed (.tar) file',
                    ),
                ),
            'doc' => '{package|package-version}
Download a package tarball.  The file will be named as suggested by the
server, for example if you download the DB package and the latest stable
version of DB is 1.2, the downloaded file will be DB-1.2.tgz.',
            ),
        );

    // }}}
    // {{{ constructor

    /**
     * PEAR_Command_Remote constructor.
     *
     * @access public
     */
    function PEAR_Command_Remote(&$ui, &$config)
    {
        parent::PEAR_Command_Common($ui, $config);
    }

    // }}}

    // {{{ remote-info

    function doRemoteInfo($command, $options, $params)
    {
/*
        return false; // coming soon

        var_dump($params[0]);
        $r = new PEAR_Remote($this->config);
        $info = $r->call('package.info', $params[0]);
        if (PEAR::isError($info)) {
            return $this->raiseError($info);
        }
        
        var_dump($info);
*/
        $r = new PEAR_Remote($this->config);
        $available = $r->call('package.listAll', true);
        if (PEAR::isError($available)) {
            return $this->raiseError($available);
        }
        $info = $available[$params[0]];
        $info["name"] = $params[0];

        $reg = new PEAR_Registry($this->config->get('php_dir'));
        $installed = $reg->packageInfo($info['name']);
        $info['installed'] = $installed['version'];
    
        $this->ui->outputData($info, $command);
        
        return true; // coming soon
    }

    // }}}
    // {{{ list-remote

    function doRemoteList($command, $options, $params)
    {
        $r = new PEAR_Remote($this->config);
        $available = $r->call('package.listAll', true);
        if (PEAR::isError($available)) {
            return $this->raiseError($available);
        }
        $i = $j = 0;
        $data = array(
            'caption' => 'Available packages:',
            'border' => true,
            'headline' => array('Package', 'Version'),
            );
        foreach ($available as $name => $info) {
            $data['data'][] = array($name, $info['stable']);
        }
        if (count($available)==0) {
            $data = '(no packages installed yet)';
        }
        $this->ui->outputData($data, $command);
        return true;
    }

    // }}}
    // {{{ list-all

    function doListAll($command, $options, $params)
    {
        $r = new PEAR_Remote($this->config);
        $reg = new PEAR_Registry($this->config->get('php_dir'));
        $available = $r->call('package.listAll', true);
        if (PEAR::isError($available)) {
            return $this->raiseError($available);
        }
        $data = array(
            'caption' => 'All packages:',
            'border' => true,
            'headline' => array('Package', 'Latest', 'Local'),
            );
                  
        foreach ($available as $name => $info) {
            $installed = $reg->packageInfo($name);
            $desc = $info['summary'];
            if (isset($params[$name]))
                $desc .= "\n\n".$info['description'];
            
            $data['data'][$info['category']][] = array(
                $name, 
                $info['stable'], 
                $installed['version'],
                $desc,
                );
        }
        $this->ui->outputData($data, $command);
        return true;
    }

    // }}}
    // {{{ search

    function doSearch($command, $options, $params)
    {
        if ((!isset($params[0]) || empty($params[0]))
            && (!isset($params[1]) || empty($params[1])))
        {
            return $this->raiseError('no valid search string suppliedy<');
        };
            
        $r = new PEAR_Remote($this->config);
        $reg = new PEAR_Registry($this->config->get('php_dir'));
        $available = $r->call('package.listAll', true);
        if (PEAR::isError($available)) {
            return $this->raiseError($available);
        }
        $data = array(
            'caption' => 'Matched packages:',
            'border' => true,
            'headline' => array('Package', 'Latest', 'Local'),
            );
            
        foreach ($available as $name => $info) {
            $found = (!empty($params[0]) && stristr($name, $params[0]) !== false);
            if (!$found && !(isset($params[1]) && !empty($params[1])
                && (stristr($info['summary'], $params[1]) !== false
                    || stristr($info['description'], $params[1]) !== false)))
            {   
                continue;
            };
                
            $installed = $reg->packageInfo($name);
            $desc = $info['summary'];
            if (isset($params[$name]))
                $desc .= "\n\n".$info['description'];
            
            $data['data'][$info['category']][] = array(
                $name, 
                $info['stable'], 
                $installed['version'],
                $desc,
                );
        }
        if (!isset($data['data'])) {
            return $this->raiseError('no packages found');
        };
        $this->ui->outputData($data, $command);
        return true;
    }

    // }}}
    // {{{ download

    function doDownload($command, $options, $params)
    {
        //$params[0] -> The package to download
        if (count($params) != 1) {
            return PEAR::raiseError("download expects one argument: the package to download");
        }
        $server = $this->config->get('master_server');
        if (!ereg('^http://', $params[0])) {
            $pkgfile = "http://$server/get/$params[0]";
        } else {
            $pkgfile = $params[0];
        }
        $this->bytes_downloaded = 0;
        $saved = PEAR_Common::downloadHttp($pkgfile, $this->ui, '.',
                                           array(&$this, 'downloadCallback'));
        if (PEAR::isError($saved)) {
            return $this->raiseError($saved);
        }
        $fname = basename($saved);
        $this->ui->outputData("File $fname downloaded ($this->bytes_downloaded bytes)", $command);
        return true;
    }

    function downloadCallback($msg, $params = null)
    {
        if ($msg == 'done') {
            $this->bytes_downloaded = $params;
        }
    }

    // }}}
    // {{{ list-upgrades

    function doListUpgrades($command, $options, $params)
    {
        include_once "PEAR/Registry.php";
        $remote = new PEAR_Remote($this->config);
        if (empty($params[0])) {
            $state = $this->config->get('preferred_state');
        } else {
            $state = $params[0];
        }
        $caption = 'Available Upgrades';
        if (empty($state) || $state == 'any') {
            $latest = $remote->call("package.listLatestReleases");
        } else {
            $latest = $remote->call("package.listLatestReleases", $state);
            $caption .= ' (' . $state . ')';
        }
        $caption .= ':';
        if (PEAR::isError($latest)) {
            return $latest;
        }
        $reg = new PEAR_Registry($this->config->get('php_dir'));
        $inst = array_flip($reg->listPackages());
        $data = array(
            'caption' => $caption,
            'border' => 1,
            'headline' => array('Package', 'Version', 'Size'),
            );
        foreach ($latest as $package => $info) {
            if (!isset($inst[$package])) {
                // skip packages we don't have installed
                continue;
            }
            extract($info);
            $inst_version = $reg->packageInfo($package, 'version');
            if (version_compare($version, $inst_version, "le")) {
                // installed version is up-to-date
                continue;
            }
            if ($filesize >= 20480) {
                $filesize += 1024 - ($filesize % 1024);
                $fs = sprintf("%dkB", $filesize / 1024);
            } elseif ($filesize > 0) {
                $filesize += 103 - ($filesize % 103);
                $fs = sprintf("%.1fkB", $filesize / 1024.0);
            } else {
                $fs = "  -"; // XXX center instead
            }
            $data['data'][] = array($package, $version, $fs);
        }
        $this->ui->outputData($data, $command);
        return true;
    }

    // }}}
}

?>