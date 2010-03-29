--TEST--
mysqli_get_client_stats()
--SKIPIF--
<?PHP
require_once('skipif.inc');
require_once('skipifemb.inc');
require_once('skipifconnectfailure.inc');
if (!function_exists('mysqli_get_client_stats')) {
	die("skip only available with mysqlnd");
}
?>
--INI--
mysqlnd.collect_statistics=1
mysqlnd.collect_memory_statistics=1
--FILE--
<?php
	/*
	TODO
	no_index_used - difficult to simulate because server/engine dependent
	bad_index_used - difficult to simulate because server/engine dependent
	flushed_normal_sets
	flushed_ps_sets
	explicit_close
	implicit_close
	disconnect_close
	in_middle_of_command_close
	explicit_free_result
	implicit_free_result
	explicit_stmt_close
	implicit_stmt_close
	*/

	function mysqli_get_client_stats_assert_eq($field, $current, $expected, &$test_counter, $desc = "") {

		$test_counter++;
		if (is_array($current) && is_array($expected)) {
			if ($current[$field] !== $expected[$field]) {
				printf("[%03d] %s Expecting %s = %s/%s, got %s/%s\n",
					$test_counter, $desc,
					$field, $expected[$field], gettype($expected[$field]),
					$current[$field], gettype($current[$field]));
			}
		} else if (is_array($current)) {
			if ($current[$field] !== $expected) {
				printf("[%03d] %s Expecting %s = %s/%s, got %s/%s\n",
					$test_counter, $desc,
					$field, $expected, gettype($expected),
					$current[$field], gettype($current[$field]));
			}
		} else {
			if ($current !== $expected) {
				printf("[%03d] %s Expecting %s = %s/%s, got %s/%s\n",
					$test_counter, $desc,
					$field, $expected, gettype($expected),
					$current, gettype($current));
			}
		}

	}

	function mysqli_get_client_stats_assert_gt($field, $current, $expected, &$test_counter, $desc = "") {

		$test_counter++;
		if (is_array($current) && is_array($expected)) {
			if ($current[$field] <= $expected[$field]) {
				printf("[%03d] %s Expecting %s = %s/%s, got %s/%s\n",
					$test_counter, $desc,
					$field, $expected[$field], gettype($expected[$field]),
					$current[$field], gettype($current[$field]));
				}
		} else {
			if ($current <= $expected) {
				printf("[%03d] %s Expecting %s = %s/%s, got %s/%s\n",
					$test_counter, $desc, $field,
					$expected, gettype($expected),
					$current, gettype($current));
			}
		}

	}


	$tmp = $link = null;
	if (!is_null($tmp = @mysqli_get_client_stats($link)))
		printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	require_once("connect.inc");

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[002] Expecting array/any_non_empty, got %s/%s\n", gettype($info), $info);

	var_dump($info);

	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[003] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
		exit(1);
	}

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[004] Expecting array/any_non_empty, got %s/%s\n", gettype($new_info), $new_info);

	if (count($info) != count($new_info)) {
		printf("[005] Expecting the same number of entries in the arrays\n");
		var_dump($info);
		var_dump($new_info);
	}

	$test_counter = 6;
	mysqli_get_client_stats_assert_gt('bytes_sent', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_gt('bytes_received', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_gt('packets_sent', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_gt('packets_received', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_gt('protocol_overhead_in', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_gt('protocol_overhead_out', $new_info, $info, $test_counter);

	// we assume the above as tested and in the following we check only those
	mysqli_get_client_stats_assert_eq('result_set_queries', $new_info, $info, $test_counter);

	/* we need to skip this test in unicode - we send set names utf8 during mysql_connect */
	if (!(version_compare(PHP_VERSION, '5.9.9', '>') == 1))
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_eq('buffered_sets', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_eq('unbuffered_sets', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_eq('ps_buffered_sets', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_eq('ps_unbuffered_sets', $new_info, $info, $test_counter);

	mysqli_get_client_stats_assert_eq('rows_skipped_ps', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_eq('copy_on_write_saved', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_eq('copy_on_write_performed', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_eq('command_buffer_too_small', $new_info, $info, $test_counter);
	// This is not a mistake that I use %unicode|string%(1) "1" here! Andrey did not go for int to avoid any
	// issues for very large numbers and 32 vs. 64bit systems
	mysqli_get_client_stats_assert_eq('connect_success', $new_info, "1", $test_counter);
	mysqli_get_client_stats_assert_eq('connect_failure', $new_info, $info, $test_counter);
	mysqli_get_client_stats_assert_eq('connection_reused', $new_info, $info, $test_counter);

	require('table.inc');
	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	// fetch stats
	$expected = $info;

	// buffered normal
	print "Testing buffered normal...\n";
	if (!$res = mysqli_query($link, 'SELECT COUNT(*) AS _num FROM test', MYSQLI_STORE_RESULT))
		printf("[%03d] SELECT COUNT() FROM test failed, [%d] %s\n",
			++$test_counter, mysqli_errno($link), mysqli_error($link));

	$expected['rows_fetched_from_server_normal'] = (string)($expected['rows_fetched_from_server_normal'] + 1);
	$expected['buffered_sets'] = (string)($expected['buffered_sets'] + 1);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);
	$expected['rows_buffered_from_client_normal'] = (string)($expected['rows_buffered_from_client_normal'] + 1);


	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_normal_buffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('buffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_normal', $info, $expected, $test_counter);

	/* no change to rows_fetched_from_client_normal_buffered! */
	if (!$row = mysqli_fetch_assoc($res))
		printf("[%03d] fetch_assoc - SELECT COUNT() FROM test failed, [%d] %s\n",
			++$test_counter, mysqli_errno($link), mysqli_error($link));

	$expected['rows_fetched_from_client_normal_buffered'] = (string)($expected['rows_fetched_from_client_normal_buffered'] + 1);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_normal_buffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_normal', $info, $expected, $test_counter);

	$num_rows = $row['_num'];
	mysqli_free_result($res);

	print "Testing buffered normal... - SELECT id, label FROM test\n";
	if (!$res = mysqli_query($link, 'SELECT id, label FROM test', MYSQLI_STORE_RESULT))
		printf("[%03d] SELECT id, label FROM test failed, [%d] %s\n",
			++$test_counter, mysqli_errno($link), mysqli_error($link));

	assert(mysqli_num_rows($res) == $num_rows);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	$expected['rows_fetched_from_server_normal'] = (string)($expected['rows_fetched_from_server_normal'] + $num_rows);
	$expected['rows_buffered_from_client_normal'] = (string)($expected['rows_buffered_from_client_normal'] + $num_rows);
	$expected['buffered_sets'] = (string)($expected['buffered_sets'] + 1);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_normal_buffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('buffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_normal', $info, $expected, $test_counter);

	/* fetching none, but stats should not be affected - current implementation */
	mysqli_free_result($res);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_normal_buffered', $info, $expected, $test_counter);

	print "Testing unbuffered normal...\n";
	if (!$res = mysqli_query($link, 'SELECT id, label FROM test', MYSQLI_USE_RESULT))
		printf("[%03d] SELECT id, label FROM test failed, [%d] %s\n",
			++$test_counter, mysqli_errno($link), mysqli_error($link));


	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_normal_unbuffered', $info, $expected, $test_counter);

	while ($row = mysqli_fetch_assoc($res))
		;
	mysqli_free_result($res);

	$expected['rows_fetched_from_server_normal'] = (string)($expected['rows_fetched_from_server_normal'] + $num_rows);
	$expected['rows_fetched_from_client_normal_unbuffered'] = (string)($expected['rows_fetched_from_client_normal_unbuffered'] + $num_rows);
	$expected['unbuffered_sets'] = (string)($expected['unbuffered_sets'] + 1);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_normal_unbuffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('unbuffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);

	print "Testing unbuffered normal... - SELECT id, label FROM test, not all fetched\n";
	if (!$res = mysqli_query($link, 'SELECT id, label FROM test', MYSQLI_USE_RESULT))
		printf("[%03d] SELECT id, label FROM test failed, [%d] %s\n",
			++$test_counter, mysqli_errno($link), mysqli_error($link));

	for ($i = 0; $i < $num_rows - 1; $i++)
		$row = mysqli_fetch_assoc($res);

	$expected['rows_fetched_from_server_normal'] = (string)($expected['rows_fetched_from_server_normal'] + $num_rows - 1);
	$expected['rows_fetched_from_client_normal_unbuffered'] = (string)($expected['rows_fetched_from_client_normal_unbuffered'] + $num_rows - 1);
	$expected['unbuffered_sets'] = (string)($expected['unbuffered_sets'] + 1);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_normal_unbuffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('unbuffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);

	print "Testing if implicit fetching and cleaning happens...\n";
	mysqli_free_result($res);

	/* last row has been implicitly cleaned from the wire by freeing the result set */
	$expected['rows_fetched_from_server_normal'] = (string)($expected['rows_fetched_from_server_normal'] + 1);
	$expected['rows_fetched_from_client_normal_unbuffered'] = (string)($expected['rows_fetched_from_client_normal_unbuffered'] + 1);
	$expected['rows_skipped_normal'] = (string)($info['rows_skipped_normal'] + 1);
	$expected['flushed_normal_sets'] = (string)($expected['flushed_normal_sets'] + 1);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_normal_unbuffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_skipped_normal', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('flushed_normal_sets', $info, $expected, $test_counter);

	print "Testing buffered Prepared Statements...\n";
	if (!$stmt = mysqli_stmt_init($link))
		printf("[%03d] stmt_init() failed, [%d] %s\n",
			++$test_counter, mysqli_errno($link), mysqli_error($link));

	if (!mysqli_stmt_prepare($stmt, 'SELECT id, label FROM test') ||
			!mysqli_stmt_execute($stmt))
		printf("[%03d] prepare/execute failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	/* by default PS is unbuffered - no change */
	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_ps', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_ps_buffered', $info, $expected, $test_counter);

	if (!mysqli_stmt_store_result($stmt))
		printf("[%03d] store_result failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	mysqli_stmt_free_result($stmt);

	$expected['rows_fetched_from_server_ps'] = (string)($expected['rows_fetched_from_server_ps'] + $num_rows);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);
	$expected['ps_buffered_sets'] = (string)($expected['ps_buffered_sets'] + 1);
	$expected['rows_buffered_from_client_ps'] = (string)($expected['rows_buffered_from_client_ps'] + $num_rows);


	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_ps', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_ps_buffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('ps_buffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_ps', $info, $expected, $test_counter);

	print "Testing buffered Prepared Statements... - fetching all\n";

	if (!mysqli_stmt_prepare($stmt, 'SELECT id, label FROM test') ||
			!mysqli_stmt_execute($stmt))
		printf("[%03d] prepare/execute failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = $label = null;
	if (!mysqli_stmt_bind_result($stmt, $id, $label))
		printf("[%03d] bind_result failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_stmt_store_result($stmt))
		printf("[%03d] store_result failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	while (mysqli_stmt_fetch($stmt))
		;

	$expected['rows_fetched_from_server_ps'] = (string)($expected['rows_fetched_from_server_ps'] + $num_rows);
	$expected['rows_fetched_from_client_ps_buffered'] = (string)($expected['rows_fetched_from_client_ps_buffered'] + $num_rows);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);
	$expected['ps_buffered_sets'] = (string)($expected['ps_buffered_sets'] + 1);
	$expected['rows_buffered_from_client_ps'] = (string)($expected['rows_buffered_from_client_ps'] + $num_rows);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_ps', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_ps_buffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('ps_buffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_ps', $info, $expected, $test_counter);

	mysqli_stmt_free_result($stmt);

	print "Testing buffered Prepared Statements... - fetching all but one\n";

	if (!mysqli_stmt_prepare($stmt, 'SELECT id, label FROM test') ||
			!mysqli_stmt_execute($stmt))
		printf("[%03d] prepare/execute failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = $label = null;
	if (!mysqli_stmt_bind_result($stmt, $id, $label))
		printf("[%03d] bind_result failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_stmt_store_result($stmt))
		printf("[%03d] store_result failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	for ($i = 0; $i < $num_rows - 1; $i++)
		mysqli_stmt_fetch($stmt);

	$expected['rows_fetched_from_server_ps'] = (string)($expected['rows_fetched_from_server_ps'] + $num_rows);
	$expected['rows_fetched_from_client_ps_buffered'] = (string)($expected['rows_fetched_from_client_ps_buffered'] + $num_rows - 1);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);
	$expected['ps_buffered_sets'] = (string)($expected['ps_buffered_sets'] + 1);
	$expected['rows_buffered_from_client_ps'] = (string)($expected['rows_buffered_from_client_ps'] + $num_rows);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_ps', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_ps_buffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('ps_buffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_ps', $info, $expected, $test_counter);

	$expected['rows_skipped_ps'] = $info['rows_skipped_ps'];
	mysqli_stmt_free_result($stmt);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	/* buffered result set - no skipping possible! */
	mysqli_get_client_stats_assert_eq('rows_skipped_ps', $info, $expected, $test_counter);

	print "Testing unbuffered Prepared Statements... - fetching all\n";

	if (!mysqli_stmt_prepare($stmt, 'SELECT id, label FROM test') ||
			!mysqli_stmt_execute($stmt))
		printf("[%03d] prepare/execute failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = $label = null;
	if (!mysqli_stmt_bind_result($stmt, $id, $label))
		printf("[%03d] bind_result failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$i = 0;
	while (mysqli_stmt_fetch($stmt))
		$i++;
	assert($num_rows = $i);

	$expected['rows_fetched_from_server_ps'] = (string)($expected['rows_fetched_from_server_ps'] + $num_rows);
	$expected['rows_fetched_from_client_ps_unbuffered'] = (string)($expected['rows_fetched_from_client_ps_unbuffered'] + $num_rows);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);
	$expected['ps_unbuffered_sets'] = (string)($expected['ps_unbuffered_sets'] + 1);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_ps', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_ps_unbuffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('ps_unbuffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_ps', $info, $expected, $test_counter);

	mysqli_stmt_free_result($stmt);

	print "Testing unbuffered Prepared Statements... - fetching all but one\n";

	if (!mysqli_stmt_prepare($stmt, 'SELECT id, label FROM test') ||
			!mysqli_stmt_execute($stmt))
		printf("[%03d] prepare/execute failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = $label = null;
	if (!mysqli_stmt_bind_result($stmt, $id, $label))
		printf("[%03d] bind_result failed, [%d] %s\n",
			++$test_counter, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	for ($i = 0; $i < $num_rows - 1; $i++)
		mysqli_stmt_fetch($stmt);

	$expected['rows_fetched_from_server_ps'] = (string)($expected['rows_fetched_from_server_ps'] + $num_rows - 1);
	$expected['rows_fetched_from_client_ps_unbuffered'] = (string)($expected['rows_fetched_from_client_ps_unbuffered'] + $num_rows - 1);
	$expected['result_set_queries'] = (string)($expected['result_set_queries'] + 1);
	$expected['ps_unbuffered_sets'] = (string)($expected['ps_unbuffered_sets'] + 1);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_ps', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_client_ps_unbuffered', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('result_set_queries', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('ps_unbuffered_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_buffered_from_client_ps', $info, $expected, $test_counter);

	mysqli_stmt_free_result($stmt);
	$expected['rows_skipped_ps'] = (string)($expected['rows_skipped_ps'] + 1);
	$expected['flushed_ps_sets'] = (string)($expected['flushed_ps_sets'] + 1);
	$expected['rows_fetched_from_server_ps'] = (string)($expected['rows_fetched_from_server_ps'] + 1);

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	mysqli_get_client_stats_assert_eq('rows_skipped_ps', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('flushed_ps_sets', $info, $expected, $test_counter);
	mysqli_get_client_stats_assert_eq('rows_fetched_from_server_ps', $info, $expected, $test_counter);

	/*
	print "Checking for normal buffered side effects...\n";
	foreach ($info as $k => $v)
		if ($info[$k] != $expected[$k])
			printf("$k - $v != %s\n", $expected[$k]);
	*/
	print "... done with fetch statistics\n";

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	//
	// result_set_queries statistics
	//

	if (!is_array($info = mysqli_get_client_stats()) || empty($info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($info), $info);

	if (!$res = mysqli_query($link, "SELECT id, label FROM test"))
		printf("[%03d] SELECT failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	$rows = 0;
	while ($row = mysqli_fetch_assoc($res))
		$rows++;

	if (0 == $rows)
		printf("[%03d] Expecting at least one result, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	mysqli_free_result($res);

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);

	mysqli_get_client_stats_assert_eq('result_set_queries', $new_info, (string)($info['result_set_queries'] + 1), $test_counter);
	$info = $new_info;

	//
	// non_result_set_queries - DDL
	//

	// CREATE TABLE, DROP TABLE
	if (!mysqli_query($link, "DROP TABLE IF EXISTS non_result_set_queries_test"))
		printf("[%03d] DROP TABLE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "CREATE TABLE non_result_set_queries_test(id INT) ENGINE = " . $engine)) {
		printf("[%03d] CREATE TABLE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));
	} else {
		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 2), $test_counter, 'CREATE/DROP TABLE');
	}
	$info = $new_info;

	// ALERT TABLE
	if (!mysqli_query($link, "ALTER TABLE non_result_set_queries_test ADD label CHAR(1)")) {
		printf("[%03d] ALTER TABLE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));
	} else {
		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'ALTER TABLE');
	}
	$info = $new_info;

	// CREATE INDEX, DROP INDEX
	if (!mysqli_query($link, "CREATE INDEX idx_1 ON non_result_set_queries_test(id)")) {
		printf("[%03d] CREATE INDEX failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));
	} else {

		if (!mysqli_query($link, "DROP INDEX idx_1 ON non_result_set_queries_test"))
			printf("[%03d] DROP INDEX failed, [%d] %s\n", ++$test_counter,
				mysqli_errno($link), mysqli_error($link));

		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 2), $test_counter, 'DROP INDEX');
	}
	$info = $new_info;

	// RENAME TABLE
	if (!mysqli_query($link, "DROP TABLE IF EXISTS client_stats_test"))
		printf("[%03d] Cleanup, DROP TABLE client_stats_test failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	$info = $new_info;

	if (!mysqli_query($link, "RENAME TABLE non_result_set_queries_test TO client_stats_test")) {
		printf("[%03d] RENAME TABLE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	} else {
		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'RENAME TABLE');

	}
	$info = $new_info;

	if (!mysqli_query($link, "DROP TABLE IF EXISTS non_result_set_queries_test"))
		printf("[%03d] Cleanup, DROP TABLE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "DROP TABLE IF EXISTS client_stats_test"))
		printf("[%03d] Cleanup, DROP TABLE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	// Let's see if we have privileges for CREATE DATABASE
	mysqli_query($link, "DROP DATABASE IF EXISTS mysqli_get_client_stats");
	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	$info = $new_info;


	// CREATE, ALTER, RENAME, DROP DATABASE
	if (mysqli_query($link, "CREATE DATABASE mysqli_get_client_stats")) {

		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'CREATE DATABASE');
		$info = $new_info;

		if (!mysqli_query($link, "ALTER DATABASE DEFAULT CHARACTER SET latin1"))
			printf("[%03d] ALTER DATABASE failed, [%d] %s\n", ++$test_counter,
				mysqli_errno($link), mysqli_error($link));

		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'CREATE DATABASE');
		$info = $new_info;

		if (mysqli_get_server_version($link) > 51700) {
			if (!mysqli_query($link, "RENAME DATABASE mysqli_get_client_stats TO mysqli_get_client_stats_"))
				printf("[%03d] RENAME DATABASE failed, [%d] %s\n", ++$test_counter,
					mysqli_errno($link), mysqli_error($link));

			if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
				printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
					++$test_counter, gettype($new_info), $new_info);
			mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'CREATE DATABASE');
			$info = $new_info;
		} else {
			if (!mysqli_query($link, "CREATE DATABASE mysqli_get_client_stats_"))
				printf("[%03d] CREATE DATABASE failed, [%d] %s\n", ++$test_counter,
					mysqli_errno($link), mysqli_error($link));
			if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
				printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
					++$test_counter, gettype($new_info), $new_info);
			$info = $new_info;
		}

		if (!mysqli_query($link, "DROP DATABASE mysqli_get_client_stats_"))
			printf("[%03d] DROP DATABASE failed, [%d] %s\n", ++$test_counter,
				mysqli_errno($link), mysqli_error($link));

		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'DROP DATABASE');
		$info = $new_info;
	}

	// CREATE SERVER, ALTER SERVER, DROP SERVER
	// We don't really try to use federated, we just want to see if the syntax works
	mysqli_query($link, "DROP SERVER IF EXISTS myself");

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	$info = $new_info;

	$sql = sprintf("CREATE SERVER myself FOREIGN DATA WRAPPER mysql OPTIONS (user '%s', password '%s', database '%s')",
		$user, $passwd, $db);
	if (mysqli_query($link, $sql)) {
		// server knows about it

		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'CREATE SERVER');
		$info = $new_info;

		if (!mysqli_query($link, sprintf("ALTER SERVER myself OPTIONS(user '%s_')", $user)))
			printf("[%03d] ALTER SERVER failed, [%d] %s\n", ++$test_counter,
				mysqli_errno($link), mysqli_error($link));

		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'ALTER SERVER');
		$info = $new_info;

		if (!mysqli_query($link, "DROP SERVER myself"))
			printf("[%03d] DROP SERVER failed, [%d] %s\n", ++$test_counter,
				mysqli_errno($link), mysqli_error($link));

		if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
			printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
		mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'DROP SERVER');
		$info = $new_info;
	}

	/*
	We don't test the NDB ones.
	13.1. Data Definition Statements
	13.1.3. ALTER LOGFILE GROUP Syntax
	13.1.4. ALTER TABLESPACE Syntax
	13.1.9. CREATE LOGFILE GROUP Syntax
	13.1.10. CREATE TABLESPACE Syntax
	13.1.15. DROP LOGFILE GROUP Syntax
	13.1.16. DROP TABLESPACE Syntax
	*/

	//
	// DML
	//
	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
				++$test_counter, gettype($new_info), $new_info);
	$info = $new_info;

	if (!mysqli_query($link, "INSERT INTO test(id) VALUES (100)"))
		printf("[%03d] INSERT failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'INSERT');
	$info = $new_info;

	if (!mysqli_query($link, "UPDATE test SET label ='z' WHERE id = 100"))
		printf("[%03d] UPDATE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'UPDATE');
	$info = $new_info;

	if (!mysqli_query($link, "REPLACE INTO test(id, label) VALUES (100, 'b')"))
		printf("[%03d] INSERT failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'REPLACE');
	$info = $new_info;

	// NOTE: this will NOT update dbl_ddls counter
	if (!$res = mysqli_query($link, "SELECT id, label FROM test WHERE id = 100"))
		printf("[%03d] SELECT@dml failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));
	mysqli_free_result($res);

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, $info, $test_counter, 'SELECT@dml');
	$info = $new_info;

	if (!mysqli_query($link, "DELETE FROM test WHERE id = 100"))
		printf("[%03d] DELETE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'DELETE');
	$info = $new_info;

	if (!$res = mysqli_query($link, "TRUNCATE TABLE test"))
		printf("[%03d] TRUNCATE failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'TRUNCATE');
	$info = $new_info;

	$file = tempnam(sys_get_temp_dir(), 'mysqli_test');
	if ($fp = fopen($file, 'w')) {
		@fwrite($fp, '1;"a"');
		fclose($fp);
		chmod($file, 0644);
		$sql = sprintf('LOAD DATA LOCAL INFILE "%s" INTO TABLE test', mysqli_real_escape_string($link, $file));
		if (mysqli_query($link, $sql)) {
			if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
				printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
					++$test_counter, gettype($new_info), $new_info);
			mysqli_get_client_stats_assert_eq('non_result_set_queries', $new_info, (string)($info['non_result_set_queries'] + 1), $test_counter, 'LOAD DATA LOCAL');
			$info = $new_info;
		}
		unlink($file);
	}

	/*
	We skip those:
	13.2. Data Manipulation Statements
	13.2.2. DO Syntax
	13.2.3. HANDLER Syntax
	13.2.5. LOAD DATA INFILE Syntax
	*/
	mysqli_query($link, "DELETE FROM test");
	if (!mysqli_query($link, "INSERT INTO test(id, label) VALUES (1, 'a'), (2, 'b')"))
		printf("[%03d] Cannot insert new records, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!$res = mysqli_real_query($link, "SELECT id, label FROM test ORDER BY id"))
		printf("[%03d] Cannot SELECT with mysqli_real_query(), [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!is_object($res = mysqli_use_result($link)))
		printf("[%03d] mysqli_use_result() failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	while ($row = mysqli_fetch_assoc($res))
		;
	mysqli_free_result($res);
	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	mysqli_get_client_stats_assert_eq('unbuffered_sets', $new_info, (string)($info['unbuffered_sets'] + 1), $test_counter, 'mysqli_use_result()');
	$info = $new_info;

	if (!$res = mysqli_real_query($link, "SELECT id, label FROM test ORDER BY id"))
		printf("[%03d] Cannot SELECT with mysqli_real_query() II, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	if (!is_object($res = mysqli_store_result($link)))
		printf("[%03d] mysqli_use_result() failed, [%d] %s\n", ++$test_counter,
			mysqli_errno($link), mysqli_error($link));

	while ($row = mysqli_fetch_assoc($res))
		;
	mysqli_free_result($res);
	if (!is_array($new_info = mysqli_get_client_stats()) || empty($new_info))
		printf("[%03d] Expecting array/any_non_empty, got %s/%s\n",
			++$test_counter, gettype($new_info), $new_info);
	mysqli_get_client_stats_assert_eq('buffered_sets', $new_info, (string)($info['buffered_sets'] + 1), $test_counter, 'mysqli_use_result()');
	$info = $new_info;

	mysqli_close($link);


	/*
	no_index_used
	bad_index_used
	flushed_normal_sets
	flushed_ps_sets
	explicit_close
	implicit_close
	disconnect_close
	in_middle_of_command_close
	explicit_free_result
	implicit_free_result
	explicit_stmt_close
	implicit_stmt_close
	*/

	print "done!";
?>
--CLEAN--
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

if (!mysqli_query($link, "DROP TABLE IF EXISTS non_result_set_queries_test"))
	printf("[c003] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

if (!mysqli_query($link, "DROP TABLE IF EXISTS client_stats_test"))
	printf("[c004] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

if (!mysqli_query($link, "DROP DATABASE IF EXISTS mysqli_get_client_stats_"))
	printf("[c005] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

if (!mysqli_query($link, "DROP DATABASE IF EXISTS mysqli_get_client_stats"))
	printf("[c006] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

if (!mysqli_query($link, "DROP SERVER IF EXISTS myself"))
	printf("[c007] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>
--EXPECTF--
array(156) {
  [%u|b%"bytes_sent"]=>
  %unicode|string%(1) "0"
  [%u|b%"bytes_received"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_sent"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_received"]=>
  %unicode|string%(1) "0"
  [%u|b%"protocol_overhead_in"]=>
  %unicode|string%(1) "0"
  [%u|b%"protocol_overhead_out"]=>
  %unicode|string%(1) "0"
  [%u|b%"bytes_received_ok_packet"]=>
  %unicode|string%(1) "0"
  [%u|b%"bytes_received_eof_packet"]=>
  %unicode|string%(1) "0"
  [%u|b%"bytes_received_rset_header_packet"]=>
  %unicode|string%(1) "0"
  [%u|b%"bytes_received_rset_field_meta_packet"]=>
  %unicode|string%(1) "0"
  [%u|b%"bytes_received_rset_row_packet"]=>
  %unicode|string%(1) "0"
  [%u|b%"bytes_received_prepare_response_packet"]=>
  %unicode|string%(1) "0"
  [%u|b%"bytes_received_change_user_packet"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_sent_command"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_received_ok"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_received_eof"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_received_rset_header"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_received_rset_field_meta"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_received_rset_row"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_received_prepare_response"]=>
  %unicode|string%(1) "0"
  [%u|b%"packets_received_change_user"]=>
  %unicode|string%(1) "0"
  [%u|b%"result_set_queries"]=>
  %unicode|string%(1) "0"
  [%u|b%"non_result_set_queries"]=>
  %unicode|string%(1) "0"
  [%u|b%"no_index_used"]=>
  %unicode|string%(1) "0"
  [%u|b%"bad_index_used"]=>
  %unicode|string%(1) "0"
  [%u|b%"slow_queries"]=>
  %unicode|string%(1) "0"
  [%u|b%"buffered_sets"]=>
  %unicode|string%(1) "0"
  [%u|b%"unbuffered_sets"]=>
  %unicode|string%(1) "0"
  [%u|b%"ps_buffered_sets"]=>
  %unicode|string%(1) "0"
  [%u|b%"ps_unbuffered_sets"]=>
  %unicode|string%(1) "0"
  [%u|b%"flushed_normal_sets"]=>
  %unicode|string%(1) "0"
  [%u|b%"flushed_ps_sets"]=>
  %unicode|string%(1) "0"
  [%u|b%"ps_prepared_never_executed"]=>
  %unicode|string%(1) "0"
  [%u|b%"ps_prepared_once_executed"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_fetched_from_server_normal"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_fetched_from_server_ps"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_buffered_from_client_normal"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_buffered_from_client_ps"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_fetched_from_client_normal_buffered"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_fetched_from_client_normal_unbuffered"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_fetched_from_client_ps_buffered"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_fetched_from_client_ps_unbuffered"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_fetched_from_client_ps_cursor"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_affected_normal"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_affected_ps"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_skipped_normal"]=>
  %unicode|string%(1) "0"
  [%u|b%"rows_skipped_ps"]=>
  %unicode|string%(1) "0"
  [%u|b%"copy_on_write_saved"]=>
  %unicode|string%(1) "0"
  [%u|b%"copy_on_write_performed"]=>
  %unicode|string%(1) "0"
  [%u|b%"command_buffer_too_small"]=>
  %unicode|string%(1) "0"
  [%u|b%"connect_success"]=>
  %unicode|string%(1) "0"
  [%u|b%"connect_failure"]=>
  %unicode|string%(1) "0"
  [%u|b%"connection_reused"]=>
  %unicode|string%(1) "0"
  [%u|b%"reconnect"]=>
  %unicode|string%(1) "0"
  [%u|b%"pconnect_success"]=>
  %unicode|string%(1) "0"
  [%u|b%"active_connections"]=>
  %unicode|string%(1) "0"
  [%u|b%"active_persistent_connections"]=>
  %unicode|string%(1) "0"
  [%u|b%"explicit_close"]=>
  %unicode|string%(1) "0"
  [%u|b%"implicit_close"]=>
  %unicode|string%(1) "0"
  [%u|b%"disconnect_close"]=>
  %unicode|string%(1) "0"
  [%u|b%"in_middle_of_command_close"]=>
  %unicode|string%(1) "0"
  [%u|b%"explicit_free_result"]=>
  %unicode|string%(1) "0"
  [%u|b%"implicit_free_result"]=>
  %unicode|string%(1) "0"
  [%u|b%"explicit_stmt_close"]=>
  %unicode|string%(1) "0"
  [%u|b%"implicit_stmt_close"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_emalloc_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_emalloc_ammount"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_ecalloc_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_ecalloc_ammount"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_erealloc_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_erealloc_ammount"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_efree_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_malloc_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_malloc_ammount"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_calloc_count"]=>
  %unicode|string%(%d) "%d"
  [%u|b%"mem_calloc_ammount"]=>
  %unicode|string%(%d) "%d"
  [%u|b%"mem_realloc_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_realloc_ammount"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_free_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_estrndup_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_strndup_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_estndup_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"mem_strdup_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_null"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_bit"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_tinyint"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_short"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_int24"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_int"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_bigint"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_decimal"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_float"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_double"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_date"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_year"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_time"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_datetime"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_timestamp"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_string"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_blob"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_enum"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_set"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_geometry"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_text_fetched_other"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_null"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_bit"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_tinyint"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_short"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_int24"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_int"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_bigint"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_decimal"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_float"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_double"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_date"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_year"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_time"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_datetime"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_timestamp"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_string"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_blob"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_enum"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_set"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_geometry"]=>
  %unicode|string%(1) "0"
  [%u|b%"proto_binary_fetched_other"]=>
  %unicode|string%(1) "0"
  [%u|b%"init_command_executed_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"init_command_failed_count"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_quit"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_init_db"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_query"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_field_list"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_create_db"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_drop_db"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_refresh"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_shutdown"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_statistics"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_process_info"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_connect"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_process_kill"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_debug"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_ping"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_time"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_delayed_insert"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_change_user"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_binlog_dump"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_table_dump"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_connect_out"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_register_slave"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_stmt_prepare"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_stmt_execute"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_stmt_send_long_data"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_stmt_close"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_stmt_reset"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_stmt_set_option"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_stmt_fetch"]=>
  %unicode|string%(1) "0"
  [%u|b%"com_deamon"]=>
  %unicode|string%(1) "0"
}
Testing buffered normal...
Testing buffered normal... - SELECT id, label FROM test
Testing unbuffered normal...
Testing unbuffered normal... - SELECT id, label FROM test, not all fetched
Testing if implicit fetching and cleaning happens...
Testing buffered Prepared Statements...
Testing buffered Prepared Statements... - fetching all
Testing buffered Prepared Statements... - fetching all but one
Testing unbuffered Prepared Statements... - fetching all
Testing unbuffered Prepared Statements... - fetching all but one
... done with fetch statistics
done!