# Memory Paging
Initially we wanted to just have memory to allocate, which is very simple to make.
The client asks for amount of pages, and the pager responds if there is enough room.
However if we want to be able to deallocate pages after we are done with them, we need to keep track of where they are in memory.
To do this we chose the simplest method we could find. char[] where each bit tells if a page is mapped or not.
Because we know where the first page starts (the page that contains the map) we can via only the address of the page find it in the array and set the bit to 0. This is fast for deallocating, although the client is responsible for keeping track of how many pages it asked for. This was done initially to avoid making another table mapping the amount of pages to the address. This could easily be done though.
Allocating memory simply loops through the array and sees if it can find enough space somewhere to allocate it. Which means linear time in terms of memory size, which is quite bad. But fine for our use case at the moment

# Issues
We mainly had issues with bit shifting things because it sucks
