program ISO7185Demo;

type
  PInteger = ^Integer;  { Define a pointer type for integers }

{ Function to calculate factorial using recursion }
function Factorial(n: Integer): Integer;
begin
  if n = 0 then
    Factorial := 1
  else
    Factorial := n * Factorial(n - 1);
end;

var
  number: Integer;      { Variable to store user input }
  result: PInteger;     { Pointer to store the result }

begin
  { Dynamically allocate memory for the result }
  New(result);

  { Prompt user for input }
  Write('Enter a number to calculate its factorial: ');
  ReadLn(number);

  { Ensure the number is non-negative }
  if number < 0 then
    WriteLn('Factorial is not defined for negative numbers.')
  else
  begin
    { Calculate factorial and store it in dynamically allocated memory }
    result^ := Factorial(number);

    { Display the result }
    WriteLn('The factorial of ', number, ' is: ', result^);
  end;

  { Free the allocated memory }
  Dispose(result);
end.
