# Making exceptions work
So reading up on the concept, we want to first configure the medeleg register to handle exceptions a certain way, then when errors happen. we check the mdedeleg register and know how to handle them.
Then we put a pointer to a bunch of assembly at the stvec register, which is run when an exception happens. the assembly calls a function that kernel panics and prints out what happened
The assembly we call primarily dumps all the registers on the stack.

## Issues
### Machine mode vs Supervisor Mode
We had issues setting the medeleg register to supervisor mode, so we initially stuck with the machine mode bit.
This means we are looking at the machine mode registers for exception handling.

### 32 vs 64 bit
Many ressources we looked at had this implemented with 32 bit, we needed to keep this in mind when building exceptions. 
