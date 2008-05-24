--TEST--
SPL: SplHeap and friends, throw: An iterator cannot be used with foreach by reference
--CREDITS--
Thomas Koch <thomas@koch.ro>
#Hackday Webtuesday 2008-05-24
--SKIPIF--
<?php if (!extension_loaded("spl")) print "skip"; ?>
--FILE--
<?php
function testForException( $heap )
{
    try
    {
        foreach( $heap as &$item );
    }
    catch( RuntimeException $e )
    {
        echo $e->getMessage(),"\n";
    }
}

// 1. SplMinHeap emtpy
$heap = new SplMinHeap;
testForException( $heap );

// 2. SplMinHeap non-emtpy
$heap = new SplMinHeap;
$heap->insert( 1 );
testForException( $heap );

// 3. SplMaxHeap emtpy
$heap = new SplMaxHeap;
testForException( $heap );

// 4. SplMaxHeap non-emtpy
$heap = new SplMaxHeap;
$heap->insert( 1 );
testForException( $heap );

// 5. SplPriorityQueue empty
$heap = new SplPriorityQueue;
testForException( $heap );

// 6. SplPriorityQueue non-empty
$heap = new SplPriorityQueue;
$heap->insert( 1, 2 );
testForException( $heap );

?>
==DONE==
--EXPECT--
An iterator cannot be used with foreach by reference
An iterator cannot be used with foreach by reference
An iterator cannot be used with foreach by reference
An iterator cannot be used with foreach by reference
An iterator cannot be used with foreach by reference
An iterator cannot be used with foreach by reference
==DONE==
