//
// array declaration and definitions
//
// string support not required
//

module array02;

var A : integer[5][5];

begin
  A[2][3] := ReadInt();
  A[0][1] := A[2][3];
  WriteInt(A[0][1]); WriteLn()
end array02.
