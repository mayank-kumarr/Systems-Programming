mkdir -p ./1.b.files.out
for FILE in ./1.b.files/*.txt;do
    sort -n $FILE>./1.b.files.out/$(basename $FILE)
done
cat ./1.b.files.out/*|sort -n|uniq -c|awk '{print $2,$1}'>./1.b.out.txt