
# Hashx86 Operating System

<p align="center">
  <img width="500" height="500" alt="Image" src="https://github.com/user-attachments/assets/e7a30385-11d8-454c-9654-6f251f28f610" />
</p>


**Status**: 🚧 This project is currently under development.

Hashx86 is a minimalistic operating system built for the **x86 architecture**. Designed primarily for educational and experimental purposes, it provides basic functionality and serves as a foundation for exploring OS concepts and low-level system programming.

Simple yet effective, Hashx86 focuses on core features without unnecessary complexity.

---

## 🔧 About

This project focuses on low-level system development, implementing core operating system functionalities such as:

- Interrupt Service Routines (ISRs)
- Physical Memory Management (PMM)
- Dynamic Kernel Heap (KHeap)
- Direct video memory manipulation (supports both VGA Text and VGA Graphics modes)
- High-resolution graphics rendering via VESA
- Hardware interaction through custom drivers
- Paging
- ELF binary loading and execution
- System call interface for custom binaries
- Multitasking with process and thread management
- Basic event handling
- Widget-based GUI framework
- GUI performance optimization using draw caching
- Support for dynamic resolution switching via dynamic drivers
- HDD driver implementation
- FAT32 filesystem support
---

## 🖼 Demonstrations

- 📺 **NEW : Dynamic BGA Graphics (High Resolution)**
  The kernel detects BGA hardware via PCI, loads the driver from the FAT32 disk and instantly switches to high resolution.

<p align="center">
  <img width="500" height="500" alt="Image" src="https://github.com/user-attachments/assets/9da5f8c1-d3a0-4293-b940-f39377d542ea" />
</p>

- 📺 **GUI Framework at 1152×864 (32-bit VESA)**  
  Showcases ELF binary execution, system call support, event handling and multitasking capabilities with threads and processes.

  https://github.com/user-attachments/assets/b311645f-1579-457b-8377-fcf03f9cf0b6

- 🧪 **Early GUI (VGA Mode 320×200)**  
  Initial implementation of the graphical interface using legacy VGA mode.

  https://github.com/user-attachments/assets/581ec179-5460-439f-b05c-82d525cdecea

- 🧩 **Interrupt Service Routines**  
  Custom ISRs tested for hardware event response and system stability.

  https://github.com/user-attachments/assets/604a1b2d-f935-476e-954c-c0c363d7e380

---

## 🚀 Ready to Build Your Own OS?

Dive deep into the fundamentals of operating system development with our comprehensive tutorial series.

**👉 [Start the Tutorial Series](https://malakagunawardana.pages.dev/workshops/build-your-own-operating-system-from-scratch/)**

Learn to build your own digital world from the ground up!

---

## 🧪 Development Roadmap

Hashx86 is currently under active development. Upcoming improvements include:

- Desktop taskbar implementation with smooth animations
- Expanded system call library
- Thread-safe process management
- Networking support
- Task State Segment (TSS) integration
- DMA / PCI driver framework
- Audio driver support
- USB driver support

**Stay tuned for future updates!**

---

## 🛠 Build Instructions

> Prerequisites:
> - GCC cross-compiler for i686
> - GRUB and `xorriso` (for ISO generation)
> - `qemu-system-i386` and `qemu-img`
> - `make`

### 1. Clone the Repository
```bash
git clone https://github.com/sdmdg/Hashx86.git
cd hashx86
```

### 2. Setup the Hard Disk
You need a `HDD.vdi` file to store drivers and files. Choose one of the methods below:

**Option A: Quick Start (Recommended)**
Download the pre-formatted `HDD.vdi` from the **Releases** page and place it in the project folder.

**Option B: Manual Setup**
If you prefer to create a fresh disk:
1. Create a 1GB VirtualBox Disk Image (VDI):
```bash
qemu-img create -f vdi HDD.vdi 1G
```

2. Run the OS for the first time. The kernel will detect the empty disk and automatically format it to **FAT32**.
```bash
make run
```
*Wait for the OS to boot and confirm formatting is complete, then close the QEMU window.*

3. Install Drivers & Assets
Now that the disk is formatted, mount it and copy the required system files (Drivers, Fonts, Bitmaps).
```bash
make hdd
```

### 3. Final Boot
Run the OS again. It will now boot with full graphics and file support!
```bash
make runq
```

---

## 📄 License

This project is licensed under the MIT License. See `LICENSE` for more details.

---

## 👤 Author

Hashx86 is developed and maintained by **[Malaka](https://github.com/sdmdg)**.  
Built with ❤️ for learning and having fun with bare-metal programming.

---

## ❤️ Special Thanks

This project wouldn’t have been possible without the help, guidance and inspiration from:

- **[Viktor Engelmann](https://www.youtube.com/watch?v=1rnA6wpF0o4&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M)** – *Write your own Operating System* YouTube series  
- **[OSDev.org](https://wiki.osdev.org/Main_Page)** – Incredible community and resources for OS development  
- **[lowlevel.eu](https://lowlevel.eu)** – Excellent tutorials on low-level programming  


## 🎨 Credits

* **Desktop Wallpaper:** "Iceland, Beach, Ice image" by **Elisabetta_Miele** on [Pixabay](https://pixabay.com/photos/iceland-beach-ice-frost-sunset-9056229/)

---
