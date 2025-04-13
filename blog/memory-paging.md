# Memory Paging

Initially we wanted to just have memory to allocate, which is very simple to
make. The client asks for amount of pages, and the pager responds if there is
enough room. However if we want to be able to deallocate pages after we are done
with them, we need to keep track of where they are in memory. To do this we
chose the simplest method we could find. char[] where each bit tells if a page
is mapped or not. Because we know where the first page starts (the page that
contains the map) we can via only the address of the page find it in the array
and set the bit to 0. This is fast for deallocating, although the client is
responsible for keeping track of how many pages it asked for. This was done
initially to avoid making another table mapping the amount of pages to the
address. This could easily be done though. Allocating memory simply loops
through the array and sees if it can find enough space somewhere to allocate it.
Which means linear time in terms of memory size, which is quite bad. But fine
for our use case at the moment

# Issues

We mainly had issues with bit shifting things because it sucks

# Heap

Heap designed to create blocks in memory. The block header is 24 bytes and
contains the total block size, including the header, a reference to the next
block in the chain, and whether the block is free. The rest of the block is then
memory to be used by processes. When data is allocated, the first free block
that fits the data is selected for the data. If the block is big enough to also
include a second block with at least 1 byte of data, the block is split in 2.

There were some issues with how the splitting was calculated, which was due to a
pointer having the wrong type (it was supposed to be a void/byte pointer, but
was a block pointer). This caused its incrementation to be in steps of
`block_size`, and not steps of `byte`.

When blocks are freed, they are merged with all free blocks that immediately
follow them. There was also an issue with this merging, causing data in
allocated blocks to be overwritten.
