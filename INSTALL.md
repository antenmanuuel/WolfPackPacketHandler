# INSTALL.md

## Installation Guide

Follow these instructions to set up your development environment. Make sure to use Ubuntu 22.04 LTS

### 1. Essential Development Packages
In a terminal window, execute the following commands to install essential development tools:

```bash
# Install gcc and man pages
sudo apt-get install -y build-essential manpages-dev

# Install the MIPS gcc cross-compiler
sudo apt-get install -y gcc-mips-linux-gnu

# Install JDK 11
sudo apt-get install -y openjdk-11-jdk-headless

# Install valgrind
sudo apt-get install -y valgrind

# Install cppcheck
sudo apt-get install -y cppcheck

# Install git
sudo apt-get install -y git 

# Install Netpbm
sudo apt-get install -y netpbm

# Install imagemagick
sudo apt-get install -y imagemagick-6.q16

# Install ncurses
sudo apt-get install -y libncurses5-dev

# Install Criterion’s dependencies
sudo apt-get install -y ninja-build meson cmake pkg-config libffi-dev libgit2-dev

# Install Criterion
sudo apt-get install -y libcriterion-dev

# Setup Visual Studio Code
curl -o vscode.deb -L http://go.microsoft.com/fwlink/?LinkID=760868
sudo apt install ./vscode.deb
rm vscode.deb

#Extensions required for Visual Studio Code
Launch VSCode: code . (yes, include the dot character)
Click on the Extensions icon (represented by 4 squares with one square displaced).
Search for and install the following extensions:
"c/c++ extension pack"
"extension pack for java"

# Install OCLint
curl -o oclint-22.02-llvm-13.0.1-x86_64-linux-ubuntu-20.04.tar.gz -L https://github.com/oclint/oclint/releases/download/v22.02/oclint-22.02-llvm-13.0.1-x86_64-linux-ubuntu-20.04.tar.gz
tar zxvf oclint-22.02-llvm-13.0.1-x86_64-linux-ubuntu-20.04.tar.gz
rm oclint-22.02-llvm-13.0.1-x86_64-linux-ubuntu-20.04.tar.gz
echo 'export PATH=/home/student/oclint-22.02/bin:$PATH' >> ~/.bashrc


