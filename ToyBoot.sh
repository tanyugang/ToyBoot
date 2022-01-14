cd $HOME/CodingMaster/EDK
export EDK_TOOLS_PATH=$HOME/CodingMaster/EDK/BaseTools
source edksetup.sh BaseTools
build
cp /home/coding/CodingMaster/EDK/ToyBoot/Build/DEBUG_GCC5/X64/ToyBoot.efi /home/coding/CodingMaster/OVMF/ESP/ToyBoot.efi
qemu-system-x86_64 -m 4096 -drive if=pflash,format=raw,file=/home/coding/CodingMaster/OVMF/OVMF_CODE.fd,readonly=on -drive if=pflash,format=raw,file=/home/coding/CodingMaster/OVMF/OVMF_VARS.fd,readonly=on -drive format=raw,file=fat:rw:/home/coding/CodingMaster/OVMF/ESP -net none
