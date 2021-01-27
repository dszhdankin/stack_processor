in
call fact
out
halt

fact:
push 0
ja rec
popd
popd
push 1
ret
rec:
popd
pop ax
push ax
push ax
push 1
sub
call fact
mul
ret


