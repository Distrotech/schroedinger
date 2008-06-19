#!/bin/sh

rm -f *.drc

for file in `find ~ -name "*.drc"`; do
    newname=`echo $file | sed -r -e s@/\.+/@@`
    ln "$file" "$newname"
done