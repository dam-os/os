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
