exts=$(find ./data1c -type f |sed 's/[.]/ /g'|awk '{print $2}'|sort|uniq)
for i in ${exts[@]};do
    mkdir -p ./data1c/"$i"
    find ./data1c -type f -name "*."$i |sort|uniq|xargs mv -t ./data1c/$i
done
mkdir -p ./data1c/"Nil"
find ./data1c -type f ! -name "*.*" |sort|uniq|xargs mv -t ./data1c/Nil
find ./data1c -type d -empty -delete