
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

---

## 🖼 Demonstrations

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

- GUI performance optimization using draw caching
- Desktop taskbar implementation with smooth animations
- Support for dynamic resolution switching
- Expanded system call library
- Thread-safe process management
- Networking support
- HDD driver implementation
- FAT32 filesystem support
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
> - QEMU or Bochs (for testing)
> - `make`

```bash
git clone https://github.com/sdmdg/Hashx86.git
cd hashx86
make run        # Build ISO image and run in QEMU
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

---