1    // Load 0
0
14   // CopyToX
4    // LoadIdxX 32 (load from A-Z table) 
32
21   // JumpIfEqual 12 
12
9    // Put 2 (output as char)
2
25   // IncX
20   // Jump 3
3
1    // Load 0
0
16   // CopyToY
5    // LoadIdxY 59 (load from 1-10 table)
59
21   // JumpIfEqual 27
27
9    // Put 1 (output as int)
1
1    // Load 1  (because no IncY instruction)
1
11   // AddY
16   // CopyToY
20   // Jump 15
15
1    // Print newline
10
9
2
50   // End
65   // Data A-Z
66
67
68
69
70
71
72
73
74
75
76
77
78
79
80
81
82
83
84
85
86
87
88
89
90
0
1    // Data 1-10
2
3
4
5
6
7
8
9
10
0

.1000
30
