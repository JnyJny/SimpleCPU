"""
"""

sample1 = """
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
"""

sample2 = """
19   // CopyFromSp
9    // Output
1
1
10
9
2
27   // push
19   // CopyFromSp
9    // Output
1
1
10
9
2
28   // pop
19   // CopyFromSp
9    // Output
1
1
10
9
2
29   // interrupt
2    // try to load from protected memory, should fail with error
1000   
50
.1000
30
.1500
19   // CopyFromSp
9    // Output
1
1
10
9
2
27   // push
19   // CopyFromSp
9    // Output
1
1
10
9
2
28   // pop
19   // CopyFromSp
9    // Output
1
1
10
9
2
30
"""

sample3 = """
23   // line one
15
23   // line two 
30
23   // line three 
51
23   // line four 
86
23   // line five 
103
23   // line six 
142
23   // line seven 
163
50

      // line one
1    ld 4
4         
27   push
23   call spaces 
206
28   remove parm 
1    ld 6
6         
27   push
23   call line
178
28   remove parm 
23   call newline
220
24   return

      // line two
1
32
9
2
1    ld /
47
9
2
1    ld 9
9         
27   push
23   call spaces 
206
28   remove parm 
1    load 
92    
9
2
23   call newline
220
24   return

      // line three
1    ld /
47
9    output
2
1    three spaces
32
9
2
9
2
9
2
23   print eye
225
1    two spaces
32
9
2
9
2
23   print eye
225
1    two spaces
32
9
2
9
2
1    load  
92    
9
2
23   call newline
220
24   return

      // line four 
1    load | 
124
9    put
2
1    ld 11
11         
27   push
23   call spaces 
206
28   remove parm 
1    load | 
124
9    put
2
23   call newline
220
24

      // line five
1    load  
92    
9
2
1    three spaces
32
9
2
9
2
9
2
1    load  
92    
9
2
1    ld 4 
4
27   push
23   call underscore 
192
28   remove parm 
1    load / 
47    
9
2
1    two spaces
32
9
2
9
2
1    load / 
47    
9
2
23   call newline
220
24   return

      // line six 
1
32
9
2
1    ld 
92
9
2
1    ld 9
9         
27   push
23   call spaces 
206
28   remove parm 
1    load / 
47 
9
2
23   call newline
220
24   return

      // line seven 
1    ld 4
4         
27   push
23   call spaces 
206
28   remove parm 
1    ld 6
6         
27   push
23   call line
178
28   remove parm 
23   call newline
220
24   return

      // print dash 
1    ld 1
1 
14   CopyToX
6    LoadSpX  - get parm
14   CopyToX
1    ld -
45
9    output
2
26   decX
15   CopyFromX
22   JNE 183
183
24

      // print underscore 
1    ld 1
1 
14   CopyToX
6    LoadSpX  - get parm
14   CopyToX
1    ld _
95
9    output
2
26   decX
15   CopyFromX
22   JNE 197
197
24

      // print space 
1    ld 1
1 
14   CopyToX
6    LoadSpX  - get parm
14   CopyToX
1    ld ' ' 
32
9    output
2
26   decX
15   CopyFromX
22   JNE 211
211
24

      // print newline 
1
10
9
2
24

      // print -* 
1    ld dash 
45       
9    output
2
1    ld asterisk
42    
9    output
2
24   return


.1000
30   interrupt handler - just return

"""

sample4 = """
.0
1    // Load 10
10
14   // CopyToX
1    // Load A
65
9    // Output A
2
1    // Load newline 
10
9    // Output newline 
2
29   // Syscall
26   // DecX
15   // CopyFromX
22   // Jump NE Load A  
3
50


.1000
27   // Push
15   // CopyFromX
27   // Push
17   // CopyFromY
27   // Push

2    // load data 
1700
14   // CopyToX
25   // IncX
15   // CopyFromX
7    // Store data 
1700

28   // Pop
16   // CopyToY
28   // Pop
14   // CopyToX
28   // Pop
30   // IRet 



.1500
27   // Push
15   // CopyFromX
27   // Push
17   // CopyFromY
27   // Push

2    // load data 
1700
9    // write value
1
1    // load newline
10
9    // write newline
2

28   // Pop
16   // CopyToY
28   // Pop
14   // CopyToX
28   // Pop
30   // IRet 

.1700
0    // data 
"""
