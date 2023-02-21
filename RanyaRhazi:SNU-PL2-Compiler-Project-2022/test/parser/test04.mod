//
// test04
//
// parser test:
// - all of test03 plus
// - integer expressions with local variables & parameters
//

module test04;

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
  b := c * (c / b + 5*a)
end foo;

begin
  a := b + 2;
  c := a * b
end test04.
