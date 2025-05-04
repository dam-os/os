rm sdcard.img
dd if=/dev/zero of=sdcard.img bs=1M count=64

parted -s sdcard.img mklabel msdos
parted -s sdcard.img mkpart primary ext4 1MiB 100%

sudo losetup -o 1048576 --sizelimit 63M /dev/loop0 sdcard.img

sudo mkfs.ext4 /dev/loop0

mkdir -p mnt
sudo mount /dev/loop0 mnt

mkimage -f my-os.its build/my-os.itb
sudo cp ./build/my-os.itb mnt/

sudo umount mnt

sudo losetup -d /dev/loop0
