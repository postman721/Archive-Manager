# Archive Manager

Archive Manager now supports QT6.

A Qt (C++) desktop application to **create and extract archives** in multiple formats  
(ZIP, TAR, TAR.GZ, TAR.BZ2, TAR.XZ) with a modern dark-themed interface.

![Archive Manager UI](https://github.com/user-attachments/assets/2cc4c89e-eeda-418c-a47e-0134344ab58d)

---

## Features

- Drag & drop files/folders to create archives  
- Open existing archives (ZIP & TAR variants)  
- Extract selected files or extract all  
- Overwrite warnings  
- Multi-selection support  
- Dark theme with customizable UI styling  

---

## Dependencies (Debian/Ubuntu – Qt 6)

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake \
  qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
  zip unzip tar
```

**Package notes**

- `qt6-base-dev` – Qt Core, GUI & Widgets libraries  
- `qt6-tools-dev` / `qt6-tools-dev-tools` – Qt tools (uic, rcc, designer)  
- `zip`, `unzip`, `tar` – Archive utilities  

---

## Build Instructions

Easiest: Use Qtcreator

### Using CMake (Recommended for Qt 6)

```bash
git clone https://github.com/yourusername/archive-manager.git
cd archive-manager

cmake -S . -B build
cmake --build build -j
./build/ArchiveManager
```

### Using qmake (Qt 6)

```bash
git clone https://github.com/yourusername/archive-manager.git
cd archive-manager

qmake6 ArchiveManager.pro
make -j$(nproc)
./ArchiveManager
```

---

## Usage

- Launch the app  
- Drag files/folders to create a new archive  
- Open an existing archive to view contents  
- Extract selected files or extract all  
- Confirm overwrite if files already exist  

---

## License

This project is licensed under the **GPL v2 License**.  
See the `LICENSE` file for details.
Copyright JJ Posti <techtimejourney.net>
