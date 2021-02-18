#!/usr/bin/env bash

declare in=$1
declare symbol=$2
if [[ "x$in" = "x" ]]; then
	echo "You must specify a input file"
	exit 1
fi

echo "char const *$symbol = \" \\"
cat $in | while read line; do
	final=$(echo $line | sed -e 's/"/\\"/g' -e s/\'/\\\\\'/g)
	echo "$final\n \\"
done
echo "\";"

