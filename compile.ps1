$CompFlags = @(
    "-pedantic"
    "-Wextra"
    "-Wno-cast-function-type"
    "-fdiagnostics-show-option"
    #"-g"
    "-O2"
)
$LinkFlags = @(
    "-pedantic"
    "-m64"
    "-mwindows"
)

windres res/res.rc -Isrc -O coff -o bin/res.obj
gcc src/*.c -Isrc $CompFlags bin/res.obj $LinkFlags -o bin/clock.exe
rm bin/res.obj