```
 _____                    
|_   _| __ __ _ _ __  ___ 
  | || '__/ _` | '_ \/ __|
  | || | | (_| | |_) \__ \
  |_||_|  \__,_| .__/|___/
               |_|        
```
## Explanation of How You Solved Level 4:

Level 0: Simply put my CS login.

Level 1: I set a break point with "break level_one", "rt" to run to the break point, and "la asm" to show the assembly code. Here we are calling "string_not_equal", which is used to compare two strings, one inside $rdi (the input), the other inside $rsi, which is the password. So I print out what is inside $rsi by "x/s $rsi" and get the password.


Level 2: It calls "read_two_numbers_greater_than_five", so I just tried 2 random input number that's greater than 5.

eax = input1, ecx = input2
ecx = ecx - 3;
edx = 2

loop {
  edx = edx * eax;
  eax = eax - 1;
}

eax = 1
edx = 2 * (eax) * (eax - 1) * (eax - 2) * ... * 1

ecx = edx
<=>
ecx - 3 = 2 * (eax) * (eax - 1) * (eax - 2) * ... * 1

Take eax = 6, ecx = 2 * 6 * 5 * 4 * 3 * 2 * 1 + 3 = 1443


Level 3:
It shouldn't jump at "<level_three+32>", otherwise a trap will be popped. According to "cmp $0xb,%rax" we know $rax - 1 <= 11, meaning the second input number should not be larger than 12. Similarly, the third input number should not be larger than 9. The first and the last input number are related. Just give a random input for the last number and observe the $rcx value which is compared at "<level_three+359>", and that should be the value for the first input number.


Level 4:
"<level_four+108>" set $edx from $eax, and $eax must be 0 in order to make test at "<level_four+115>" return 0; otherwise, it will fall into trap. I printed out and see $rsi is "trap", so $rdi must be the same word in order for "<strings_not_equal>" to return 0. There is a loop start at "<level_four+51>", and it sets $rax to 0 and keep looping until $rax is 4, and every time we are extracting 4 more bytes from $rdx, producing "bdta", "arutbdta", "gaotarutbdta", "psaigaotarutbdta" for each of the 4 iterations respectively. 

Before "movzbl" at "<level_four+81>", $rcx stands for how many bytes we want to keep for the current 4-byte string (counting from right, so for instance $rdx = "bdta" and $rcx = 1 means we want to keep "ta"), and it's calculated from $rsp + $rax * 8. $rsp is essentially the input, and the 4 input numbers correspond to $rsp, $rsp + 8, $rsp + 16, $rsp + 24 respectively. "<level_four+69>" gives the constraint that all input numbers must be less than 3 so that it won't move over the current 4 byte. 

After "movzbl", $ecx stands for the string after truncation (i.e. if the current string is "bdta" and correct letter is 't', then it will be truncated to "ta" and stored in $ecx). Then $cl will take the left-most byte in $ecx and output it to 0x2b(%rsp,%rax) at "<level_four+85>", which will be the correct letter for current position.

Having those observations, now the task simply becomes finding how many bytes we want to keep for each of the 4-byte string. So for instance, we know the first 4-byte string is "bdta" and the first letter in $rdi should be 't', so the first number we input should be 1.

The idea above can be illustrated by this:

--------------
|     'p'    |  <-- $rsp + 0x2b + 3
--------------
|     'a'    |  <-- $rsp + 0x2b + 2
--------------
|     'r'    |  <-- $rsp + 0x2b + 1
--------------
|     't'    |  <-- $rsp + 0x2b
--------------
|     ...    | 
--------------
|      3     |  <-- $rsp + 24
--------------
|      2     |  <-- $rsp + 16
--------------
|      2     |  <-- $rsp + 8
--------------
|      1     |  <-- $rsp
--------------
|     ...    | 
