in
call fib
out
halt

fib:
push 1
ja rec
popd
popd
push 1
ret

rec:

popd
pop ax
push ax
push 1
sub
push ax
push 2
sub

call fib
pop bx
pop ax
push bx
push ax
call fib
add
ret