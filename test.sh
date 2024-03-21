for i in {1..100000}
do
    ./build/compiler -riscv hello.c -o hello.koopa
    if [ $? -ne 0 ]; then
    echo "Command failed, exiting loop"
    break
  fi
done