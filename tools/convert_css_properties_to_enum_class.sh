#!/usr/bin/env bash

#cat /usr/include/libcss/properties.h | grep 'CSS_' | grep -v 'CSS_PROP' >tmp.txt 
cat /usr/include/libcss/properties.h | grep 'CSS_' | grep -v 'CSS_PROP' | cut -d '=' -f 1 | sed -e 's/\t//g' -e 's/ //g' >tmp.txt

declare cur_match_prop=''
cat tmp.txt | while read line; do
	if echo $line | grep 'INHERIT' 2>&1 >/dev/null; then
		if [[ "x$cur_match_prop" != "x" ]]; then
			echo "};"
			echo
		fi
		cur_match_prop=`echo $line | sed -e 's/_INHERIT//g'`
		p=$(echo $cur_match_prop | sed -e "s/CSS//g" -e 's/\([A-Z]\)/\L\0/g' -e 's/_\([a-z]\)/\U\1/g')
		echo "enum class PV${p}"
		echo "{"
	fi

	value_name=`echo $line | sed -e "s/${cur_match_prop}_//g"| perl -ple 's/([A-Z])/\L$1\E/g'`
	enum_member=`echo _$value_name | sed -e 's/_\([a-z]\)/\U\1/g'`
	echo "    k$enum_member,"
#	echo "$cur_match_prop: $enum_member"
done
