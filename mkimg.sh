# Create an empty disk image (adjust size as needed)
dd if=/dev/zero of=sdcard.img bs=1M count=64

# Format it with an ext4 filesystem
mkfs.ext4 sdcard.img

# Mount the image
mkdir -p mnt
sudo mount sdcard.img mnt

# Copy your kernel to the image
sudo cp ./build/kernel.elf mnt/
# You could also copy any other files your OS needs

# Unmount
sudo umount mnt
