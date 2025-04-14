  .equ BUF_SIZE, 100
  .equ BUF_SIZE_m_1, BUF_SIZE - 1  
  
  .section .data
debug_fmt:          .string "|%s|\n"
sum_fmt:            .string "%d\n"
err_empty:          .string "error: empty string\n"
err_missing_space:  .string "error: missing space between arguments\n"
err_empty_num:      .string "error: one of the numbers is empty\n"
err_more_args:      .string "error: more than two arguments\n"
err_invalid_first:  .string "error: the first number is incorrect\n"
err_invalid_second: .string "error: the second number is incorrect\n"
err_overflow:       .string "error: buffer overflow\n"

  .section .bss
buf:               .space 100

  .section .text
.globl main
main:
  la   a0, buf
  li   a1, BUF_SIZE
  call gets
  la   t0, buf

rm_newline:
  lb   t1, 0(t0)
  beq  t1, zero, done_rm
  li   t2, '\n'
  beq  t1, t2, newline_found
  li   t2, '\r'
  beq  t1, t2, newline_found
  addi t0, t0, 1
  j    rm_newline

newline_found:
  sb   zero, 0(t0)  

done_rm:
  la   a0, debug_fmt
  la   a1, buf
  call printf

  la   a0, buf
  lb   t0, 0(a0)
  beq  t0, zero, empty_string

  la   t0, buf
  li   t1, 0

count_loop:
  lb   t2, 0(t0)
  beq  t2, zero, count_done
  addi t1, t1, 1
  addi t0, t0, 1
  j    count_loop
count_done:
  li   t3, BUF_SIZE_m_1
  bne  t1, t3, no_overflow
  la   t0, buf
  add t0, t0, t1
  lb   t2, -1(t0)
  bne  t2, zero, overflow_buf

no_overflow:
  la   t0, buf
find_space:
  lb   t1, 0(t0)
  beq  t1, zero, missing_space
  li   t2, ' '
  beq  t1, t2, found_space
  addi t0, t0, 1
  j    find_space

empty_string:
  li   a0, 2
  la   a1, err_empty
  call fprintf
  li   a0, 1
  call exit

missing_space:
  li   a0, 2
  la   a1, err_missing_space
  call fprintf
  li   a0, 1
  call exit

overflow_buf:
  li   a0, 2
  la   a1, err_overflow
  call fprintf
  li   a0, 1
  call exit

found_space:
  sb   zero, 0(t0)
  addi t3, t0, 1

  mv   t4, t3

check_more:
  lb   t5, 0(t4)
  beq  t5, zero, check_empty_nums
  li   t6, ' '
  beq  t5, t6, more_args
  addi t4, t4, 1
  j    check_more

more_args:
  li   a0, 2
  la   a1, err_more_args
  call fprintf
  li   a0, 1
  call exit

check_empty_nums: 
  la   a0, buf
  lb   s2, 0(a0)
  beq  s2, zero, empty_num
  lb   s3, 0(t3)
  beq  s3, zero, empty_num
  la   a0, buf

check_first:
  lb   t0, 0(a0)
  beq  t0, zero, check_second
  li   t1, '0'
  blt  t0, t1, invalid_first
  li   t1, '9'
  bgt  t0, t1, invalid_first
  addi a0, a0, 1
  j    check_first

check_second:
  mv   a0, t3  
check_second_loop:
  lb   t0, 0(a0)
  beq  t0, zero, convert
  li   t1, '0'
  blt  t0, t1, invalid_second
  li   t1, '9'
  bgt  t0, t1, invalid_second
  addi a0, a0, 1
  j    check_second_loop

convert:
  la   a0, buf
  call atoi
  mv   s0, a0

  mv   a0, t3
  call atoi
  mv   s1, a0

  add  a1, s0, s1

  la   a0, sum_fmt
  call printf
  li   a0, 0
  call exit

empty_num:
  li   a0, 2
  la   a1, err_empty_num
  call fprintf
  li   a0, 1
  call exit

invalid_first:
  li   a0, 2  
  la   a1, err_invalid_first
  call fprintf
  li   a0, 1
  call exit

invalid_second:
  li   a0, 2
  la   a1, err_invalid_second
  call fprintf
  li   a0, 1
  call exit
