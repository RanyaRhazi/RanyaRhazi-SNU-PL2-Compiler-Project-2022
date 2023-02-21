//
// error01 
//
// parser test: syntax errors
// - module identifier does not match
// - function identifier does not match
// - procedure with a return type
// - function without a return type
// - extra semicolon at end of statement sequence
// - module body missing
//
// The errors must be eliminated one by one for testing since the compiler
// aborts at the first error.
//

module error01;

// change "foox" -> "foo" to eliminate
function foo(): integer;
begin
end foox;

// change "procedure" -> "function" to eliminate
procedure bar: integer;
begin
end bar;

// add return type to eliminate
function foobar();
begin
  return 1;  // remove semicolon to eliminate
end foobar;

// add "begin" to eliminate
end error0x. // change to "error01" to eliminate
