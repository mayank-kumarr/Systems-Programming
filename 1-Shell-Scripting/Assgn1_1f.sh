awk -v column=$2 '{print tolower($column)}' $1|sort|uniq -c|awk '{print $2,$1}'|sort -k2 -rno 1c_output_$2_column.freq