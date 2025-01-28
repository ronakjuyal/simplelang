.text

ldi A 30
sta %c
lda %c
ldi B 20
cmp
je %IF_TRUE_0
jmp %END_IF_0
IF_TRUE_0:
lda %c
ldi B 40
add
sta %c
END_IF_0:
out 0
hlt

.data

c = 0
