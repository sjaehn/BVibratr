A .wvt is a text file for wavetable data as floating point values. 

Rules:

1) Each line of the text file has to contain either a parameter, or a value, or a comment or nothing  (empty line). 

2) A parameter name is followed by its value. The only supported parameter yet
is SAMPLES_PER_FRAME. The default value for SAMPLES_PER_FRAME is 128.

3) The wavetable sample values have to be provided as positive or negative
floating point numbers in a non-scientific notation with decimal points or
decimal commas. The values should not exceed [-1, 1].

4) A comment starts with # end ends at the end of the line.

5) Leading and tailing whitespaces are ignored.

6) The total number for samples should be divisible by SAMPLES_PER_FRAME
Otherwise overhanging data are ignored.

7) The wavetable data shall continue from the beginning after the last sample
without breaks. 


```
# A minimal wavetable with data of a 20 points sine wave

SAMPLES_PER_FRAME 20

0
0.309016994374947
0.587785252292473
0.809016994374948
0.951056516295154
1
0.951056516295154
0.809016994374948
0.587785252292473
0.309016994374948
0
-0.309016994374947
-0.587785252292473
-0.809016994374947
-0.951056516295154
-1
-0.951056516295154
-0.809016994374948
-0.587785252292473
-0.309016994374948
```