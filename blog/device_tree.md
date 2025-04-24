# Device tree

David had already written device tree, but it just printed to the terminal.

All memory-mapped registers, and hardware info, used in the OS are currently
hardcoded. We discover them either by reading sources on the internet, or by
reading the Device Tree. Device Tree includes useful info. We currently print
it, scroll through it until we find a specific device, then copy the hex value
for either the memory address or other info.

We want to rewrite Device Tree code so that we can dynamically find these
values.

We introduce structs to keep device tree nodes, and their properties. A Node can
have multiple properties as well as child nodes.

We usually only want to inspect a single node, so we keep searching through the
device tree until we find the first node that matches our search pattern.

Since the devices are recursive we can create recursive functions that step
through each section of the device tree.

While rewriting it to be recursive, issues were found in the way memory
allocation functioned.

- krealloc didn't work, so it was removed in favor of manually allocating and
  freeing memory in device tree
- free had some issues with merging blocks, so that functionality was removed
- kalloc has some issues with deciding when to split blocks, so that was fixed

When these were fixed, device tree still had its own issues:

- double_or_init function did not work, as it had swapped the src and dst args
  for memcpy.
- it seemed to skip every other device tree token
  - this seems to have caused it to also skip the *_END tokens, causing parsing
    issues.
  - This was happening due to an incorrect implementation of align_pointer,
    which added 4 to the pointer even if it was already at a 4-byte boundary.
- the values given to values of device properties were incorrect.
  - This was likely because the values were not endian-swapped
  - turns out this wasn't an issue - the values were just pointers into the
    device tree space. It is the consumers job to decode the values, as we
    cannot do it beforehand without dynamically allocating value space, which I
    guess we could do, but it would waste memory.

Device tree is now used by all code parts that require memory mapped data. They
find their required memory addresses using device tree. There is a current issue
with the implementation however. It requires loading the device tree nodes into
memory, thus allocating space for them. Since printing relies on device tree to
know which address to interact with, we are essentially unable to print anything
during memory paging and allocation stages, so a method should be added to get a
device tree address without requiring malloc
