--TEST--
SPL: SplMaxHeap: large unordered input iterated
--FILE--
<?php
$input = range(1,100);
shuffle($input);

$h = new SplMaxHeap();

foreach($input as $i) {
    $h->insert($i);
}

foreach ($h as $k => $o) {
    echo "$k => $o\n";
}
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
100 => 100
99 => 99
98 => 98
97 => 97
96 => 96
95 => 95
94 => 94
93 => 93
92 => 92
91 => 91
90 => 90
89 => 89
88 => 88
87 => 87
86 => 86
85 => 85
84 => 84
83 => 83
82 => 82
81 => 81
80 => 80
79 => 79
78 => 78
77 => 77
76 => 76
75 => 75
74 => 74
73 => 73
72 => 72
71 => 71
70 => 70
69 => 69
68 => 68
67 => 67
66 => 66
65 => 65
64 => 64
63 => 63
62 => 62
61 => 61
60 => 60
59 => 59
58 => 58
57 => 57
56 => 56
55 => 55
54 => 54
53 => 53
52 => 52
51 => 51
50 => 50
49 => 49
48 => 48
47 => 47
46 => 46
45 => 45
44 => 44
43 => 43
42 => 42
41 => 41
40 => 40
39 => 39
38 => 38
37 => 37
36 => 36
35 => 35
34 => 34
33 => 33
32 => 32
31 => 31
30 => 30
29 => 29
28 => 28
27 => 27
26 => 26
25 => 25
24 => 24
23 => 23
22 => 22
21 => 21
20 => 20
19 => 19
18 => 18
17 => 17
16 => 16
15 => 15
14 => 14
13 => 13
12 => 12
11 => 11
10 => 10
9 => 9
8 => 8
7 => 7
6 => 6
5 => 5
4 => 4
3 => 3
2 => 2
1 => 1
===DONE===
