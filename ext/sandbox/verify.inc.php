<?php
function checkname($name) {
    if (strlen($name) < 3)
        return 4;
    if (strlen($name) > 30)
        return 3;
    if (!preg_match('/^[a-zA-Z0-9._-]+$/', $name))
        return 2;
    if (app_count('username', $name))
        return 1;
    return 0;
}

function checkemail($email) {
    if (strlen($email) > 50)
        return 4;
    if (!preg_match('/^[a-zA-Z0-9._-]+@[a-zA-Z._-]+$/', $email))
        return 2;
    if (!preg_match('/^[a-zA-Z0-9._-]+@(mail.)?ustc.edu(.cn)?$/', $email))
        return 3;
    if (!check_email_count($email))
        return 1;
    return 0;
}

function checkfolder($folder) {
    if (strlen($folder) < 3)
        return 5;
    if (strlen($folder) > 30)
        return 3;
    if (!preg_match('/^[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]$/', $folder))
        return 1;
    if (!preg_match('/^[a-z0-9-]+$/', $folder))
        return 4;
    if (in_array($folder, array('www','app','admin','test','blog','example')))
        return 6;
    if (app_count('appname', $folder))
        return 2;
    return 0;
}
