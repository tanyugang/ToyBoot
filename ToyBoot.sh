cd $HOME/ToyOS/EDK
export EDK_TOOLS_PATH=$HOME/ToyOS/EDK/BaseTools
source edksetup.sh BaseTools
build
cp $HOME/ToyOS/EDK/ToyBoot/Build/DEBUG_GCC/X64/ToyBoot.efi $HOME/ToyOS/OVMF/ESP/ToyBoot.efi
qemu-system-x86_64 -m 4096 -drive if=pflash,format=raw,file=$HOME/ToyOS/OVMF/OVMF_CODE.fd,readonly=on -drive if=pflash,format=raw,file=$HOME/ToyOS/OVMF/OVMF_VARS.fd,readonly=on -drive format=raw,file=fat:rw:$HOME/ToyOS/OVMF/ESP -net none
