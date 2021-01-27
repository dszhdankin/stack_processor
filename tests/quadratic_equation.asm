in
in
in
call solve
call print
halt

print:
loop_start:
push 0
jbe end
popd
push 1
sub
pop cx
out
push cx
jmp loop_start
end:
popd
popd
ret

solve:
pop cx
pop bx
push 0
je a_zero
popd
pop ax
push bx
push bx
mul
push 4
push ax
push cx
mul
mul
sub
push 0
ja sqrt
je zero
popd
popd
push 0
ret
zero:
popd
popd
push bx
push -1
mul
push ax
push 2
mul
div
push 1
ret
sqrt:
popd
sqrt
pop [10]
push bx
push -1
mul
push [10]
add
push 2
push ax
mul
div
push bx
push -1
mul
push [10]
sub
push 2
push ax
mul
div
push 2
ret
a_zero:
popd
popd
push cx
push bx
call solve_linear

solve_linear:
push 0
je linear_zero
popd
pop bx
push -1
mul
push bx
div
push 1
ret
linear_zero:
popd
popd
push 0
je inf_solutions
popd
popd
push 0
ret
inf_solutions:
popd
popd
push 1
push 0
div
push 1
ret