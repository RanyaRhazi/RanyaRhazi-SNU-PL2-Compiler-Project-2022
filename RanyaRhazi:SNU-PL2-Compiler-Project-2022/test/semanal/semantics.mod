//
// semantics.mod
//
// semantic anaysis
// - constants assignments (range, type, unary operators)
// - variable definitions (use before def, multiple definitions)
// - function parameters (number, type)
// - type checking
//   - expressions: compatible operations/operands
//   - assignments: type of LHS = type of RHS
//   - return statements: correct type
//   - boolean types for conditions
//   - arrays

// - does not test strings

module semantics;


var u,v: boolean;             // pass
    d,e: char;                // pass
    x,y: integer;             // pass
    a  : integer[10];         // pass
    b  : integer[5][10];      // pass
    str: char[256];           // pass
    d  : integer;             // fail - duplicated identifier
    t1 : integer[];           // fail - open arrays not allowed here


// empty procedure
procedure ProcedureCall();
begin
end ProcedureCall;

// empty function. This is considered correct code
// even though there is no return statement.
function FunctionCall(): integer;
begin
end FunctionCall;


// constant assignments and unary +/-/! operations
procedure Constants;
var a,b: boolean;
    c,d: char;
    i,j: integer;
    l,k: longint;
    s  : char[14];
begin
  // boolean variables
  a := true;                  // pass
  a := false;                 // pass
  a := !true;                 // pass
  a := !!!true;               // pass
  a := !false;                // pass
  a := -true;                 // fail - type mismatch
  a := +false;                // fail - type mismatch
  a := 'a';                   // fail - type mismatch ('incompatible types in assignment)
  a := 0;                     // fail - type mismatch ('incompatible types in assignment)

  // character variables
  c := 'a';                   // pass
  d := '\n';                  // pass
  c := +'0';                  // fail - type mismatch
  c := true;                  // fail - type mismatch ('incompatible types in assignment)
  d := 0;                     // fail - type mismatch ('incompatible types in assignment)

  // integer variables
  i := -0;                    // pass
  i := +0;                    // pass
  i := --0;                   // fail - syntax error (factor expected)
  i := +-0;                   // fail - syntax error
  i := -2147483648;           // pass (min integer)
  j :=  2147483647;           // pass (max integer)
  i := -2147483649;           // fail (<min integer)
  j :=  2147483648;           // fail (>max integer)
  i := true;                  // fail - type mismatch ('incompatible types in assignment)
  i := 'a';                   // fail - type mismatch ('incompatible types in assignment)

  // longint variables
  l := 0L;                    // pass
  k := -5L;                   // pass
  l := 2L * k;                // pass
  l := 0;                     // fail - type mismatch (if implicit widening is not supported)
  k := k * 2;                 // fail - type mismatch (if implicit widening is not supported)
  l := a;                     // fail - type mismatch ('incompatible types in assignment)
  l := c;                     // fail - type mismatch ('incompatible types in assignment)
  l := i;                     // fail - type mismatch (if implicit widening is not supported)
  l := 9223372036854775807L;  // pass (max longint)
  k := -9223372036854775808L; // pass (min longint)
  k := 9223372036854775808L;  // fail (> max longint)
  l := -9223372036854775809L; // fail (< min longint)

  return
end Constants;


// variable definitions
procedure UseBeforeDef(k: integer);
var i,j: integer;
begin
  i := j;                     // pass
  i := k;                     // pass
  i := x;                     // pass
  i := z;                     // fail - syntax error (undefined identifier)

  return
end UseBeforeDef;


// multiple definitions that cause syntax errors
procedure MultipleDef(i: integer; i: integer); // fail (parameter - parameter)
var i: integer;               // fail (parameter - local var)
var m,m: integer;             // fail (local var - local var)
begin
  i := 1
end MultipleDef;


// parameters: too many/few/wrong type of parameters
procedure Parameters(p1, p2: integer);
begin
  Parameters();               // fail
  Parameters(1);              // fail
  Parameters(true, false);    // fail
  Parameters(1,2,3);          // fail
  Parameters(1,2,3,4);        // fail

  Parameters(p2,p1);          // pass
  Parameters(1,2)             // pass
end Parameters;


// type checks
procedure Expressions(p1, p2, p3, p4: integer);
var a,b,c: boolean;
    i,j,k: integer;
    A    : integer[10];
begin
  a := 1 + true;              // fail
  a := true + false;          // fail
  a := b + c;                 // fail
  a := a > b;                 // fail
  a := !!!b;                  // pass
  a := a && (!b);             // pass

  i := j + b;                 // fail
  i := j && k || p1;          // fail
  i := !j;                    // fail
  i := j + -k;                // fail - syntax error
  i := j + (-k);              // pass

  a := a && !b && (i < j)     // pass
         && (j < k)
         || (i = k);

  return
end Expressions;


// type checks involving arraysk
procedure ArrayA();
var a: boolean;
    i: integer;
    A: integer[10];
    B: integer[10][20];
    D: char[5];
    E: boolean[20];
begin
  a := i[0];                  // fail - not an array ('invalid array expression')
  a[0] := true;               // fail - not an array ('invalid array expression')
  A[0] := i;                  // pass
  A[-1] := B[0][0];           // pass
  A[0][0] := i;               // fail - dimension error ('invalid array expression')
  B[0][0] := i;               // pass
  D[1] := A[0];               // fail - type mismatch
  E[0] := B[A[0]][A[1]] > 0;  // pass

  return
end ArrayA;

procedure ArrayB(A: integer[][5]);
begin
end ArrayB;

procedure ArrayC(A: integer[][][]);
var B: integer[5][5];
    C: integer[5][5][5];
    D: integer[7][7];
begin
  ArrayB(B);                  // pass
  ArrayB(A[1]);               // fail - subarrays cannot be passed ('incomplete array expression (sub-arrays are not supported)')
  ArrayB(C[0]);               // fail - subarrays cannot be passed
  ArrayB(D);                  // fail - dimension mismatch ('argument type mismatch')

  return
end ArrayC;

// assignment type checks
procedure Assignments(p1, p2, p3, p4: integer);
var a,b,c: boolean;
    i,j,k: integer;
begin
  a := i;                     // fail
  i := a;                     // fail
  i := ProcedureCall();       // fail

  return
end Assignments;


// return statements type checks
procedure ProcReturn();
begin
  return 5                    // fail
end ProcReturn;

function NoReturn(): integer;
begin
  return                      // fail - expression expected after return
end NoReturn;

function IntReturn(): integer;
begin
  return 1 > 2                // fail - return type mismatch
end IntReturn;

function BoolReturn(): boolean;
begin
  return 1 + 2                // fail - return type mismatch
end BoolReturn;


// condition type checking
procedure If(p1, p2: integer);
begin
  if (p1 + p2 > 0) then       // fail
    return
  else
    return
  end;

  if (ProcedureCall()) then   // fail - boolean expression expected
//  if (FunctionCall()>0) then  // pass
    return
  end
end If;

procedure While();
var a,b: integer;
begin
  while (a) do                // fail - boolean expression expected
    b := b-1
  end
end While;


begin
  u := v;                   // pass
  x := y+1                  // pass
end semantics.
