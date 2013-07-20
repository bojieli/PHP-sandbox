<?php

namespace Symfony\Component\Process;

class Process
{
    const ERR = 'err';
    const OUT = 'out';

    const STATUS_READY = 'ready';
    const STATUS_STARTED = 'started';
    const STATUS_TERMINATED = 'terminated';

    const STDIN = 0;
    const STDOUT = 1;
    const STDERR = 2;

    private $commandline;
    private $cwd;
    private $env;
    private $stdin;
    private $timeout;
    private $options;
    private $exitcode;
    private $processInformation;
    private $stdout;
    private $stderr;
    private $enhanceWindowsCompatibility;
    private $pipes;
    private $process;
    private $status = self::STATUS_READY;

    private $fileHandles;
    private $readBytes;

    public static $exitCodes = array(
        0 => 'OK',
        1 => 'General error',
        2 => 'Misuse of shell builtins',

        126 => 'Invoked command cannot execute',
        127 => 'Command not found',
        128 => 'Invalid exit argument',

        // signals
        129 => 'Hangup',
        130 => 'Interrupt',
        131 => 'Quit and dump core',
        132 => 'Illegal instruction',
        133 => 'Trace/breakpoint trap',
        134 => 'Process aborted',
        135 => 'Bus error: "access to undefined portion of memory object"',
        136 => 'Floating point exception: "erroneous arithmetic operation"',
        137 => 'Kill (terminate immediately)',
        138 => 'User-defined 1',
        139 => 'Segmentation violation',
        140 => 'User-defined 2',
        141 => 'Write to pipe with no one reading',
        142 => 'Signal raised by alarm',
        143 => 'Termination (request to terminate)',
        // 144 - not defined
        145 => 'Child process terminated, stopped (or continued*)',
        146 => 'Continue if stopped',
        147 => 'Stop executing temporarily',
        148 => 'Terminal stop signal',
        149 => 'Background process attempting to read from tty ("in")',
        150 => 'Background process attempting to write to tty ("out")',
        151 => 'Urgent data available on socket',
        152 => 'CPU time limit exceeded',
        153 => 'File size limit exceeded',
        154 => 'Signal raised by timer counting virtual time: "virtual timer expired"',
        155 => 'Profiling timer expired',
        // 156 - not defined
        157 => 'Pollable event',
        // 158 - not defined
        159 => 'Bad syscall',
    );

    public function __construct($commandline, $cwd = null, array $env = null, $stdin = null, $timeout = 60, array $options = array())
    {
        if (!function_exists('proc_open')) {
            throw new \RuntimeException('The Process class relies on proc_open, which is not available on your PHP installation.');
        }

        $this->commandline = $commandline;
        $this->cwd = null === $cwd ? getcwd() : $cwd;
        if (null !== $env) {
            $this->env = array();
            foreach ($env as $key => $value) {
                $this->env[(binary) $key] = (binary) $value;
            }
        } else {
            $this->env = null;
        }
        $this->stdin = $stdin;
        $this->setTimeout($timeout);
        $this->enhanceWindowsCompatibility = true;
        $this->options = array_replace(array('suppress_errors' => true, 'binary_pipes' => true), $options);
    }

    public function __destruct()
    {
        // stop() will check if we have a process running.
        $this->stop();
    }

    public function run($callback = null)
    {
        $this->start($callback);

        return $this->wait($callback);
    }

    public function start($callback = null)
    {
        if ($this->isRunning()) {
            throw new \RuntimeException('Process is already running');
        }

        $this->stdout = '';
        $this->stderr = '';
        $callback = $this->buildCallback($callback);

        //Fix for PHP bug #51800: reading from STDOUT pipe hangs forever on Windows if the output is too big.
        //Workaround for this problem is to use temporary files instead of pipes on Windows platform.
        //@see https://bugs.php.net/bug.php?id=51800
        /*if (defined('PHP_WINDOWS_VERSION_BUILD')) {
            $this->fileHandles = array(
                self::STDOUT => tmpfile(),
            );
            $this->readBytes = array(
                self::STDOUT => 0,
            );
            $descriptors = array(array('pipe', 'r'), $this->fileHandles[self::STDOUT], array('pipe', 'w'));
        } else {*/
            $descriptors = array(array('pipe', 'r'), array('pipe', 'w'), array('pipe', 'w'));
        //}

        $commandline = $this->commandline;

        if (defined('PHP_WINDOWS_VERSION_BUILD') && $this->enhanceWindowsCompatibility) {
            $commandline = 'cmd /V:ON /E:ON /C "'.$commandline.'"';
            if (!isset($this->options['bypass_shell'])) {
                $this->options['bypass_shell'] = true;
            }
        }

        $this->process = proc_open($commandline, $descriptors, $this->pipes, $this->cwd, $this->env, $this->options);

        if (!is_resource($this->process)) {
            throw new \RuntimeException('Unable to launch a new process.');
        }
        $this->status = self::STATUS_STARTED;

        foreach ($this->pipes as $pipe) {
            stream_set_blocking($pipe, false);
        }

        if (null === $this->stdin) {
            fclose($this->pipes[0]);
            unset($this->pipes[0]);

            return;
        }

        $writePipes = array($this->pipes[0]);
        unset($this->pipes[0]);
        $stdinLen = strlen($this->stdin);
        $stdinOffset = 0;

        while ($writePipes) {
            /*if (defined('PHP_WINDOWS_VERSION_BUILD')) {
                $this->processFileHandles($callback);
            }*/

            $r = $this->pipes;
            $w = $writePipes;
            $e = null;

            $n = @stream_select($r, $w, $e, $this->timeout);

            if (false === $n) {
                break;
            }
            if ($n === 0) {
                proc_terminate($this->process);

                throw new \RuntimeException('The process timed out.');
            }

            if ($w) {
                $written = fwrite($writePipes[0], (binary) substr($this->stdin, $stdinOffset), 8192);
                if (false !== $written) {
                    $stdinOffset += $written;
                }
                if ($stdinOffset >= $stdinLen) {
                    fclose($writePipes[0]);
                    $writePipes = null;
                }
            }

            foreach ($r as $pipe) {
                $type = array_search($pipe, $this->pipes);
                $data = fread($pipe, 8192);
                if (strlen($data) > 0) {
                    call_user_func($callback, $type == 1 ? self::OUT : self::ERR, $data);
                }
                if (false === $data || feof($pipe)) {
                    fclose($pipe);
                    unset($this->pipes[$type]);
                }
            }
        }

        $this->updateStatus();
    }

    public function wait($callback = null)
    {
        $this->updateStatus();
        $callback = $this->buildCallback($callback);
        while ($this->pipes || (defined('PHP_WINDOWS_VERSION_BUILD') && $this->fileHandles)) {
            if (defined('PHP_WINDOWS_VERSION_BUILD') && $this->fileHandles) {
                $this->processFileHandles($callback, !$this->pipes);
            }

            if ($this->pipes) {
                $r = $this->pipes;
                $w = null;
                $e = null;

                $n = @stream_select($r, $w, $e, $this->timeout);

                if (false === $n) {
                    $this->pipes = array();

                    continue;
                }
                if (0 === $n) {
                    proc_terminate($this->process);

                    throw new \RuntimeException('The process timed out.');
                }

                foreach ($r as $pipe) {
                    $type = array_search($pipe, $this->pipes);
                    $data = fread($pipe, 8192);
                    if (strlen($data) > 0) {
                        call_user_func($callback, $type == 1 ? self::OUT : self::ERR, $data);
                    }
                    if (false === $data || feof($pipe)) {
                        fclose($pipe);
                        unset($this->pipes[$type]);
                    }
                }
            }
        }
        $this->updateStatus();
        if ($this->processInformation['signaled']) {
            throw new \RuntimeException(sprintf('The process stopped because of a "%s" signal.', $this->processInformation['stopsig']));
        }

        $time = 0;
        while ($this->isRunning() && $time < 1000000) {
            $time += 1000;
            usleep(1000);
        }

        $exitcode = proc_close($this->process);

        if ($this->processInformation['signaled']) {
            throw new \RuntimeException(sprintf('The process stopped because of a "%s" signal.', $this->processInformation['stopsig']));
        }

        return $this->exitcode = $this->processInformation['running'] ? $exitcode : $this->processInformation['exitcode'];
    }

    public function getOutput()
    {
        $this->updateOutput();

        return $this->stdout;
    }

    public function getErrorOutput()
    {
        $this->updateErrorOutput();

        return $this->stderr;
    }

    public function getExitCode()
    {
        $this->updateStatus();

        return $this->exitcode;
    }

    public function getExitCodeText()
    {
        $this->updateStatus();

        return isset(self::$exitCodes[$this->exitcode]) ? self::$exitCodes[$this->exitcode] : 'Unknown error';
    }

    public function isSuccessful()
    {
        $this->updateStatus();

        return 0 == $this->exitcode;
    }

    public function hasBeenSignaled()
    {
        $this->updateStatus();

        return $this->processInformation['signaled'];
    }

    public function getTermSignal()
    {
        $this->updateStatus();

        return $this->processInformation['termsig'];
    }

    public function hasBeenStopped()
    {
        $this->updateStatus();

        return $this->processInformation['stopped'];
    }

    public function getStopSignal()
    {
        $this->updateStatus();

        return $this->processInformation['stopsig'];
    }

    public function isRunning()
    {
        if (self::STATUS_STARTED !== $this->status) {
            return false;
        }

        $this->updateStatus();

        return $this->processInformation['running'];
    }

    public function stop($timeout=10)
    {
        $timeoutMicro = (int) $timeout*10E6;
        if ($this->isRunning()) {
            proc_terminate($this->process);
            $time = 0;
            while (1 == $this->isRunning() && $time < $timeoutMicro) {
                $time += 1000;
                usleep(1000);
            }

            foreach ($this->pipes as $pipe) {
                fclose($pipe);
            }
            $this->pipes = array();

            $exitcode = proc_close($this->process);
            $this->exitcode = -1 === $this->processInformation['exitcode'] ? $exitcode : $this->processInformation['exitcode'];

            if (defined('PHP_WINDOWS_VERSION_BUILD')) {
                foreach ($this->fileHandles as $fileHandle) {
                    fclose($fileHandle);
                }
                $this->fileHandles = array();
            }
        }
        $this->status = self::STATUS_TERMINATED;

        return $this->exitcode;
    }

    public function addOutput($line)
    {
        $this->stdout .= $line;
    }

    public function addErrorOutput($line)
    {
        $this->stderr .= $line;
    }

    public function getCommandLine()
    {
        return $this->commandline;
    }

    public function setCommandLine($commandline)
    {
        $this->commandline = $commandline;
    }

    public function getTimeout()
    {
        return $this->timeout;
    }

    public function setTimeout($timeout)
    {
        if (null === $timeout) {
            $this->timeout = null;

            return;
        }

        $timeout = (integer) $timeout;

        if ($timeout < 0) {
            throw new \InvalidArgumentException('The timeout value must be a valid positive integer.');
        }

        $this->timeout = $timeout;
    }

    public function getWorkingDirectory()
    {
        return $this->cwd;
    }

    public function setWorkingDirectory($cwd)
    {
        $this->cwd = $cwd;
    }

    public function getEnv()
    {
        return $this->env;
    }

    public function setEnv(array $env)
    {
        $this->env = $env;
    }

    public function getStdin()
    {
        return $this->stdin;
    }

    public function setStdin($stdin)
    {
        $this->stdin = $stdin;
    }

    public function getOptions()
    {
        return $this->options;
    }

    public function setOptions(array $options)
    {
        $this->options = $options;
    }

    public function getEnhanceWindowsCompatibility()
    {
        return $this->enhanceWindowsCompatibility;
    }

    public function setEnhanceWindowsCompatibility($enhance)
    {
        $this->enhanceWindowsCompatibility = (Boolean) $enhance;
    }

    protected function buildCallback($callback)
    {
        $that = $this;
        $out = self::OUT;
        $err = self::ERR;
        $callback = function ($type, $data) use ($that, $callback, $out, $err) {
            if ($out == $type) {
                $that->addOutput($data);
            } else {
                $that->addErrorOutput($data);
            }

            if (null !== $callback) {
                call_user_func($callback, $type, $data);
            }
        };

        return $callback;
    }

    protected function updateStatus()
    {
        if (self::STATUS_STARTED !== $this->status) {
            return;
        }

        $this->processInformation = proc_get_status($this->process);
        if (!$this->processInformation['running']) {
            $this->status = self::STATUS_TERMINATED;
            if (-1 !== $this->processInformation['exitcode']) {
                $this->exitcode = $this->processInformation['exitcode'];
            }
        }
    }

    protected function updateErrorOutput()
    {
        if (isset($this->pipes[self::STDERR]) && is_resource($this->pipes[self::STDERR])) {
            $this->addErrorOutput(stream_get_contents($this->pipes[self::STDERR]));
        }
    }

    protected function updateOutput()
    {
        if (defined('PHP_WINDOWS_VERSION_BUILD') && isset($this->fileHandles[self::STDOUT]) && is_resource($this->fileHandles[self::STDOUT])) {
            fseek($this->fileHandles[self::STDOUT], $this->readBytes[self::STDOUT]);
            $this->addOutput(stream_get_contents($this->fileHandles[self::STDOUT]));
        } elseif (isset($this->pipes[self::STDOUT]) && is_resource($this->pipes[self::STDOUT])) {
            $this->addOutput(stream_get_contents($this->pipes[self::STDOUT]));
        }
    }

    private function processFileHandles($callback, $closeEmptyHandles = false)
    {
        $fh = $this->fileHandles;
        foreach ($fh as $type => $fileHandle) {
            fseek($fileHandle, $this->readBytes[$type]);
            $data = fread($fileHandle, 8192);
            if (strlen($data) > 0) {
                $this->readBytes[$type] += strlen($data);
                call_user_func($callback, $type == 1 ? self::OUT : self::ERR, $data);
            }
            if (false === $data || ($closeEmptyHandles && '' === $data && feof($fileHandle))) {
                fclose($fileHandle);
                unset($this->fileHandles[$type]);
            }
        }
    }
}
