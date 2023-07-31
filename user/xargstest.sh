mkdir a
echo hello1 > a/b
mkdir c
echo hello2 > c/b
echo hello3 > b
find . b | xargs grep hello
