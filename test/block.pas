label
    123, 456;
const
    A = -42;
    B = 43;
    C = 44;
    D = 3.14;
    E = 2E-3;
    F = -3.14e-17;
    G = 'a';
    H = '!';
    I = '''';
    J = 'Test';
    MyValue = -DoesNotExist;
type
    Number = integer;
    RealNumber = real;
    Letter = char;
    Sentence = string;
    Predicate = boolean;
    Distance = ThisTypeDoesNotExist;
    Colors = (Red, Green, Blue);
    Range1 = 1..10;
    Range2 = -10..+10;
    Range3 = red..green;
    Range4 = '0'..'9';
    Array1 = array [1..100] of real;
    Array2 = array [Boolean] of Color;
    Array3 = array [Boolean] of array [1..10] of array [size] of real;
    Array4 = array [Boolean] of array [1..10, size] of real;
    Array5 = array [Boolean, 1..10, size] of real;
    Array6 = array [Boolean, 1..10] of array [size] of real;
    Array7 = packed array [1..10, 1..8] of boolean;
    Array8 = packed array [1..10] of packed array[1..8] of boolean;
    MyRecord = record
        a: integer;
        case isCool: boolean of
            true, 1, 2: (b: real);
            false: ();
            'test': ();
    end;
    myWonderfulSet = set of integer;
    setTest = set of 1..10;
    anotherSet = set of Colors;
    suit = packed set of (club, diamond, heart, spade);
    TPerson = record
        name: array[1..100] of char;
        case hobby: THobby of
            None: ();
            Tennis: (
                favoriteCourt: array[1..100] of char;
                case rating: integer of
                    0, 1, 2, 3: (textual: array[1..100] of char);
                    4, 5, 6: ();
            );
    end;
    TFile = file of integer;
    PFile = ^TFile;
    PInteger = @integer;
var
    clausKleber: TPerson;
    a, b, c: integer;
    scientificValue: real;
    isCool: boolean;
    myNewlyDefinedArrayType: array[5..42] of real;
    myNewlyDefinedRecordType: record
        a: integer;
        b: real;
        case boolean of
            true: (c: char);
            false: ();
    end;
    k: 0..9;
    operator: (plus, minus, times);
    m, m1, m2: array [1..10, 1..10] of real;
    MyFile: file of integer;
    ComplicatedFile: file of record
        a: integer;
        b: real;
        c: char;
        case boolean of
            true: (d: boolean);
            false: ();
    end;
    myPointer: ^integer;
    myPointer: ^real;
    myPointer: ^boolean;
    myPointer: ^char;
    myFilePointer: @ComplicatedFile;
