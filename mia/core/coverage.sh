#!/bin/sh

uncovered_all=0
code_all=0 


libfiles=$(ls *.cc *.cxx| grep -v ^test_)

for f in $libfiles; do 
    gcov CMakeFiles/miacore.dir/ $f 2>/dev/null 1>/dev/null 
done 


files=$(ls *.hh *.cc *.cxx| grep -v ^test_)

for f in $files; do 
    code=$(grep "^ *[0-9#]*:" $f.gcov 2>/dev/null | wc -l) 
    uncovered=$(grep "^ *#*:" $f.gcov 2>/dev/null | wc -l)
    echo $f $uncovered of $code
    uncovered_all=$(expr $uncovered_all + $uncovered)
    code_all=$(expr $code_all + $code)
done 

echo Summary: $(expr 100 \* \( $code_all - $uncovered_all \) / $code_all)% covered 

