### Archive Manager 

A C++ (Qt-based) desktop application to create and extract archives in various formats (ZIP, TAR, TAR.GZ, TAR.BZ2, TAR.XZ) with a modern dark-themed interface.

<img width="955" height="666" alt="Image" src="https://github.com/user-attachments/assets/2cc4c89e-eeda-418c-a47e-0134344ab58d" />

#### Features of  Archive Manager(beta, 3rd of August 2025).


- Drag & Drop files/folders to create archives.

- Open existing archives (ZIP & TAR variants) to list contents.

- Extract selected files or entire archives.

- Archive creation works with all formats, including zip.

- Overwrite warnings when target files/folders already exist.

- Multi-selection support for batch operations.

- Dark theme with customizable UI styling.


#### Debian Dependencies as an example

Install required packages on Debian/Ubuntu:

		sudo apt-get update

		sudo apt-get install build-essential qtbase5-dev qttools5-dev-tools unzip zip tar

- build-essential: Compiler toolchain (gcc, g++, make)

- qtbase5-dev: Qt Core and GUI development libraries

- qttools5-dev-tools: Qt tools (UIC, RCC, etc.)

- unzip: List and extract ZIP archives

- zip: Create ZIP archives - feature coming on future release.

- tar: Handle TAR archives (incl. gz, bz2, xz)


#### Build Instructions

Clone the repository:

		git clone https://github.com/yourusername/archive-manager.git

		cd archive-manager

Generate Makefile using qmake or CMake:

With qmake:

		qmake ArchiveManager.pro

With CMake:

		mkdir build && cd build

		cmake ..

Build:

		make -j$(nproc)

Run the executable:

		./ArchiveManager



#### Usage

- Launch the app.

- Drag files/folders into the list to prepare a new archive, or click Open to browse and load an existing archive.

- To create an archive, choose a target format and filename; confirm overwrite if file exists.

- To extract, select entries and Extract File(s) or use Extract All; confirm overwrite if target exists.

License

This project is licensed under the GPL v2 License. See LICENSE for details.

