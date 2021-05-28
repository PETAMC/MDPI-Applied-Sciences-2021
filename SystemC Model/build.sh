#!/usr/bin/env bash


# Finding SystemC-Installation
echo -e -n "\e[1;34mChecking for SystemC: "
SystemCDirectories=(
"/opt/systemc"
"/opt/systemc-2.3.3"
"/usr/local/systemc-2.3.3"
)

for directory in "${SystemCDirectories[@]}" ;
do
    if [[ -d "$directory" ]] ; then
        echo -e "\e[1;32mFound\e[1;30m at $directory"
        SYSTEMC="$directory"
        break
    fi
done
if [[ -z "$SYSTEMC" ]] ; then
    echo -e "\e[1;31mNot found!"
    exit 1
fi


# Check for tinyxml2
echo -e -n "\e[1;34mChecking for TinyXML 2: "
TinyXML2=$(pkg-config --libs tinyxml2 2> /dev/null)
if [[ -z "$TinyXML2" ]] ; then
    echo -e "\e[1;31mNot found!"
    exit 1
else
    echo -e "\e[1;32mFound"
fi


# Check for python
echo -e -n "\e[1;34mChecking for Python 3: "
pkg-config --exists python3-embed 2> /dev/null
if [[ $? == 0 ]] ; then
    PythonCFlags=$(pkg-config --cflags python3-embed)
    PythonLFlags=$(pkg-config --libs   python3-embed)
else
    PythonCFlags="$(python3-config --cflags )"
    PythonLFlags="$(python3-config --ldflags)"
fi

if [[ -z "PythonLFlags" ]] ; then
    echo -e "\e[1;31mNot found!"
    exit 1
else
    echo -e "\e[1;32mFound"
fi


# Check for GSL
echo -e -n "\e[1;34mChecking for GSL: "
pkg-config --exists gsl
if [[ $? != 0 ]] ; then
    echo -e "\e[1;31mNot found!"
    exit 1
else
    GSLCFlags="$(pkg-config --cflags gsl)"
    GSLLFlags="$(pkg-config --libs   gsl)"
    echo -e "\e[1;32mFound"
fi


# Checking for compilers
echo -e -n "\e[1;34mChecking for LLVM Compilers: "
if ! type "clang++" 2> /dev/null > /dev/null ; then
    echo -e "\e[1;31mNot found!"
    exit 1
else
    CPP="clang++"
    C="clang"
    echo -e "\e[1;32mFound"
fi


# Prepare compiling process
SOURCE=$(find . -type f -name "*.cpp")
APPS="./apps"
CPPFLAGS="-g3 -O2 -std=c++14 $PythonCFlags -I. -I$SYSTEMC/include $GSLCFlags"
LIBS="-L$SYSTEMC/lib-linux64 -lsystemc $GSLLFlags $TinyXML2 $PythonLFlags -L$APPS -ldl -lpthread"


# Compile Code
for c in $SOURCE ;
do
    echo -e "\e[1;35m ‣ \e[1;34mCompiling $c …\e[0m"
    $CPP $CPPFLAGS -c -o "${c%.*}.o" $c
    if [[ $? == 0 ]] ; then
        echo -e -n "\e[s\e[1A\e[1C\e[1;32m✔\e[u"
    fi
done

echo -e "\e[1;34mCompiling special files …\e[0m"
$C -O2 -c -o ./apps/image.o ./apps/image.c

echo -e "\e[1;34mLinking model …\e[0m"
OBJECTS=$(find . -type f -name "*.o")

$CPP -o model $OBJECTS $LIBS
rm $OBJECTS

echo -e "\e[1;32mdone\e[0m"
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

