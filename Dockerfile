FROM archlinux:base-devel AS base

# Perform initial system update
RUN pacman -Syu --noconfirm

# Install base requirements for development
RUN pacman -S --noconfirm gdb arm-none-eabi-gcc arm-none-eabi-newlib cmake git

# Install extra packages
RUN pacman -S --noconfirm wget   # Required for pulling JLink GDB server
RUN pacman -S --noconfirm neovim # Include neovim for editing
RUN pacman -S --noconfirm ccache # Include ccache

# Pull JLink GDB server package 
RUN wget --post-data "accept_license_agreement=accepted" https://www.segger.com/downloads/jlink/JLink_Linux_V812b_x86_64.tgz
# Unpack and remove tarball
RUN tar -xzf JLink_Linux_V812b_x86_64.tgz && rm -r JLink_Linux_V812b_x86_64.tgz
# Add to path
ENV PATH="/JLink_Linux_V812b_x86_64:${PATH}"

# Install CPM into cmake/ directory
RUN mkdir -p cmake
RUN wget -O cmake/CPM.cmake https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
