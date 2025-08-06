# Hashx86 Operating System

**Status**: 🚧 This project is currently under development.

Hashx86 is a minimalistic operating system built for the **x86 architecture**. Designed primarily for educational and experimental purposes, it provides basic functionality and serves as a foundation for exploring OS concepts and low-level system programming.

Simple yet effective, Hashx86 focuses on core features without unnecessary complexity.

---

## 🔧 About

This project focuses on low-level system development, implementing core operating system functionalities such as:

- Direct video memory manipulation (Supports VGA-Text and VGA-Graphics modes)
- High-resolution graphics rendering via **VESA**
- Hardware interaction via custom drivers
- ELF binary loading and execution
- System call interface
- Paging
- Multitasking with process and thread management
- Basic event handling through custom ISRs (Interrupt Service Routines)
- Widget-based GUI framework

---

## 🖼 Demonstrations

- 📺 **GUI Framework at 1152×864 (32-bit VESA)**  
  Showcases ELF binary execution, system call support, event handling and multitasking capabilities with threads and processes.

  https://github.com/user-attachments/assets/b4d59fac-6c18-4394-ab95-c3f524193a17

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

- GUI performance enhancements using draw caching
- Expanded system call set
- Thread-safe process management
- Filesystem support
- TSS

Stay tuned for future updates!

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
make iso        # Build ISO image
make run        # Run in QEMU
```

---

## 📄 License

This project is licensed under the MIT License. See `LICENSE` for more details.

---

## 👤 Author

Hashx86 is developed and maintained by **Malaka**.  
Built with ❤️ for learning and having fun with bare-metal programming.
