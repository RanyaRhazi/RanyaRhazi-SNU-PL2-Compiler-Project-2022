//
// test05
//
// parser test:
// - all of test04 plus
// - function and procedure calls
//

module test05;

var a: integer;
    b,c: integer;

function bar(a: integer): integer;
var b: integer;
begin
  return a + b
end bar;

procedure foo();
var a: integer;
begin
  a := b + (3 * c - 5);
  b := c * (c / b + 5*a);
  c := bar(3*b)
end foo;

begin
  a := b + 2;
  c := a * b;
  foo()
end test05.
