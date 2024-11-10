```
 ____         __  __
| __ ) _   _ / _|/ _| ___ _ __
|  _ \| | | | |_| |_ / _ \ '__|
| |_) | |_| |  _|  _|  __/ |
|____/ \__,_|_| |_|  \___|_|
```
I have added some comments as explanations in the 4 txt files, but I will give some explanations here, too.
<br><br>
Level 1:
From "getbuf" we know $rsp is substracted 96 bytes (0x60), and buffer ($rdi) is 80 bytes (0x50) below $rbp. So we fill the buffer with 80 bytes (random values) and 8 bytes padding (random values), and finally 8 bytes of the address of "lights_off" function.
<br><br>
Level 2:
Very similar to Level 2, we fill the buffer with 80 bytes (random values) and 8 bytes padding (random values), and finally 8 bytes of the address of "sandwich_order" function. Now we also need to pass in the arguments to the funtion. So we first add 8 bytes fake return address, then 8-byte representation of sandwich_order.id (the cookie), then 4 values of the array (sandwich_order.sammich_types).
<br><br>
Level 3:
Here we need to return the cookie instead of 1, so the first thing is moving the cookie into the return register ($eax), and then push the return address (the line after the call to "getbuf" in "test_exploit") onto the stack. Then we need to get the value of old $rbp by using gdb to print the value of $rbp before getting into "getbuf". Then we need to get the overwritten address (the address of the start of the exploit code) by using gdb to print the value of $rdi in "getbuf". Finally, fill the remaining 67 bytes of buffer with random values. 
<br><br>
Level 4:
Very similar to Level 3, except at this time the address of $rsp and $rdi are not fixed. Noticing the offset between $rbp and $rsp is fixed (0x20, 32 bytes), we represent old $rbp with $rsp. To get the value of $rdi, we need to run gdb 5 times and take the maximum value of $rdi to ensure it will get into nops and go all the way until the exploit code. From "getbufn" we know buffer ($rdi) is 544 bytes (0x220) below $rbp. The explot code takes 18 bytes, so we then need to fill the rest 526 bytes with nops.
