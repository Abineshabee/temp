>>> Required Tools
$ sudo apt install build-essential nasm xorriso grub-pc-bin 
$ sudo apt install nasm 
$ sudo apt install qemu-kvm -y
$ sudo apt install gcc-i686-linux-gnu binutils-i686-linux-gnu
$ i686-linux-gnu-gcc --version
$ i686-linux-gnu-ld --version
$ sudo apt install grub-pc-bin xorriso
$ sudo apt install mtools

>>> Create Work Place
$ mkdir -p myos/{boot,iso}
$ cd myos

>>> Assemble Bootloader
$ nasm -f elf32 boot.asm -o boot.o

>>> Compile Kernel
$ i686-linux-gnu-gcc -ffreestanding -m32 -c kernel.c -o kernel.o

>>> Link Everything
$ i686-linux-gnu-ld -T linker.ld -o kernel.bin kernel.o boot.o

>>> ISO file ( Bootable ISO )
$ mkdir -p iso/boot/grub
$ cp kernel.bin iso/boot/kernel.bin
$ nano iso/boot/grub/grub.cfg
'''
set timeout=0
set default=0

menuentry "MyOS" {
    multiboot /boot/kernel.bin
    boot
}
'''

>>> Retry ISO Creation
$ grub-mkrescue -o myos.iso iso

>>> Run in QEMU
$ qemu-system-i386 -cdrom myos.iso

>>> Running Your ( myos.iso ) in VirtualBox 

Now that you have `myos.iso`, follow these steps to boot it in **Oracle VirtualBox** on Ubuntu.  


### **🔹 1️⃣ Open VirtualBox & Create a New VM**
1. **Open VirtualBox** and click **"New"** to create a new virtual machine.
2. **Set Name** → `MyOS` (or any name you like).
3. **Choose Type** → `Other`.
4. **Choose Version** → `Other/Unknown (64-bit or 32-bit, based on your OS)`.
5. Click **Next**.

---

### **🔹 2️⃣ Configure Hardware**
1. **Memory (RAM):** Set at least `128 MB` (or more if needed).
2. **Hard Disk:** Select **Do not add a virtual hard disk**.
3. Click **Create**.

---

### **🔹 3️⃣ Attach the ISO File**
1. Select your **MyOS VM** and click **Settings**.
2. Go to **Storage**.
3. Under **Controller: IDE**, click **Empty**.
4. On the right, click the **CD icon** → **Choose a disk file...**.
5. Select `myos.iso`.
6. Click **OK**.

---

### **🔹 4️⃣ Enable EFI (If Needed)**
1. Go to **Settings** → **System**.
2. **Enable EFI** if your OS requires it.

---

### **🔹 5️⃣ Boot the OS**
1. Select the **MyOS VM**.
2. Click **Start**.
3. If everything is correct, your OS should boot! 🎉  

---

### **Troubleshooting**
- **Black screen?** Check if you selected the correct architecture (32-bit/64-bit).  
- **Boot error?** Make sure `grub.cfg` is correctly set up.  
- **ISO not found?** Ensure `myos.iso` is in the correct directory and properly mounted.  


### **Controller Type in VirtualBox (for ISO Booting)**
The **controller type** in VirtualBox determines how storage devices (like hard disks and CD/DVD drives) are connected.  

For booting an **ISO file**, you typically use:  
- **Controller Type: IDE (Recommended for ISOs)**
- **Alternative: SATA (For newer OS support, but not needed for simple boot ISOs)**  

---

### **How to Set the Correct Controller Type**
#### **1️⃣ Check Your Current Controller Type**
1. Open **VirtualBox** and select your VM (`MyOS`).
2. Click **Settings → Storage**.
3. Look under **Controller** (e.g., **IDE Controller** or **SATA Controller**).

#### **2️⃣ Change Controller Type (If Needed)**
1. If you see a **SATA Controller** and need IDE:
   - Click **Controller: SATA** → **Remove Controller** (if no disk is attached).
   - Click **Add Controller** → **Choose IDE Controller**.
   - Click **Add Optical Drive** → **Select your ISO** (`myos.iso`).
2. If you are using **IDE Controller**:
   - Under **Controller: IDE**, select the **CD/DVD drive**.
   - Click the **CD icon** → **Choose a disk file...**.
   - Select your ISO.

#### **3️⃣ Save & Boot**
- Click **OK** to save changes.
- Start the VM.

---

### **🔹 Best Controller Type for Different Use Cases**
| **Use Case**                                | **Recommended Controller** |
|---------------------------------------------|----------------------------|
| Booting from ISO (Live OS)                  | **IDE** (Recommended)      |
| Installing a modern OS (Linux, Windows 10+) | **SATA**                   |
| High-speed disk performance                 | **NVMe or SCSI**           |




