# SnuPL/-1
SnuPL/-1 defines a simple grammar consisting of assignments to 'variables' formed by expressions of constant digits using addition, subtraction, multiplication, and division. Expressions can be compared for equality (=) or inequality (#). All variable names are lower-caps and are only one character long. Separate assignment statements are separated by a semicolon (;). The end of the program is marked by a dot.

[[_TOC_]]

## EBNF
    module       = statSequence "."
    digit        = "0".."9".
    letter       = "a".."z".
    factOp       = "*" | "/".
    termOp       = "+" | "-".
    relOp        = "=" | "#".
    factor       = digit | "(" expression ")".
    term         = factor { factOp factor }.
    simpleexpr   = term { termOp term }.
    expression   = simpleexpr [ relOp simpleexpr ].
    assignment   = letter ":=" expression.
    statement    = assignment.
    statSequence = [ statement { ";" statement } ].
    whitespace   = { " " | \n }+.

## Examples
A valid SnuPL/-1 program.

    a := 5 + (1 * 5 / 2) - 2;

    b := (1 + 1);
    c := (2 * 3) = (3 * 2);

    d    :=1      +2/3*    2=    5
    .

The token stream of the above program is
```
  1:1: tLetter (a)
  1:3: tAssign
  1:6: tDigit (5)
  1:8: tPlusMinus (+)
  1:10: tLBrak
  1:11: tDigit (1)
  1:13: tMulDiv (*)
  1:15: tDigit (5)
  1:17: tMulDiv (/)
  1:19: tDigit (2)
  1:20: tRBrak
  1:22: tPlusMinus (-)
  1:24: tDigit (2)
  1:25: tSemicolon
  3:1: tLetter (b)
  3:3: tAssign
  3:6: tLBrak
  3:7: tDigit (1)
  3:9: tPlusMinus (+)
  3:11: tDigit (1)
  3:12: tRBrak
  3:13: tSemicolon
  4:1: tLetter (c)
  4:3: tAssign
  4:6: tLBrak
  4:7: tDigit (2)
  4:9: tMulDiv (*)
  4:11: tDigit (3)
  4:12: tRBrak
  4:14: tRelOp (=)
  4:16: tLBrak
  4:17: tDigit (3)
  4:19: tMulDiv (*)
  4:21: tDigit (2)
  4:22: tRBrak
  4:23: tSemicolon
  6:1: tLetter (d)
  6:6: tAssign
  6:8: tDigit (1)
  6:15: tPlusMinus (+)
  6:16: tDigit (2)
  6:17: tMulDiv (/)
  6:18: tDigit (3)
  6:19: tMulDiv (*)
  6:24: tDigit (2)
  6:25: tRelOp (=)
  6:30: tDigit (5)
  7:1: tDot
  8:1: tEOF
```


Here is an example with invalid statements:
    
    a := 1 +/ 2 3; 
    b := 1 = 2 = 3;      // only one relOp is allowed
    cd := 12 + 13;       // variables/numbers must be one character/digit only
    
The SnuPL/-1 scanner outputs the following
```
  1:1: tLetter (a)
  1:3: tAssign
  1:6: tDigit (1)
  1:8: tPlusMinus (+)
  1:9: tMulDiv (/)
  1:11: tDigit (2)
  1:13: tDigit (3)
  1:14: tSemicolon
  2:1: tLetter (b)
  2:3: tAssign
  2:6: tDigit (1)
  2:8: tRelOp (=)
  2:10: tDigit (2)
  2:12: tRelOp (=)
  2:14: tDigit (3)
  2:15: tSemicolon
  2:22: tMulDiv (/)
  2:23: tMulDiv (/)
  2:25: tLetter (o)
  2:26: tLetter (n)
  ...
  2:35: tLetter (e)
  2:36: tLetter (l)
  2:37: tUndefined (invalid character 'O')
  2:38: tLetter (p)
  ...
  2:48: tLetter (e)
  2:49: tLetter (d)
  3:1: tLetter (c)
  3:2: tLetter (d)
  3:4: tAssign
  3:7: tDigit (1)
  3:8: tDigit (2)
  3:10: tPlusMinus (+)
  3:12: tDigit (1)
  3:13: tDigit (3)
  3:14: tSemicolon
  3:22: tMulDiv (/)
  3:23: tMulDiv (/)
  3:25: tLetter (v)
  ...
  3:73: tLetter (l)
  3:74: tLetter (y)
  4:1: tEOF
```
Note how identifiers and numbers consisting of several characters/digits do not lead to an error but are recognized as individual tokens.
