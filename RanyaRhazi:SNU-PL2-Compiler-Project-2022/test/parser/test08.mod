//
// test08
//
// parser test:
// - strings
// - built-in routines
//   - WriteStr() WriteLn()
//   - WriteInt() and ReadInt()
//

module test08;

var age, year: integer;

begin
  WriteStr("Enter your age: "); age := ReadInt();
  WriteStr("Enter the year: "); year := ReadInt();

  WriteStr("You will be 100 years old in the year ");
  WriteInt(year + 100 - age);
  WriteStr(".\n")
end test08.
