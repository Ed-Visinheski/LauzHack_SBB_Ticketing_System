#!/usr/bin/env bash
# setup.sh - Install Qt 6 and latest clang on Ubuntu (apt) or macOS (brew)
# Usage: ./setup.sh [ubuntu|macos|all]
set -euo pipefail

OS_ARG="${1:-}"
SCRIPT_NAME="$(basename "$0")"

err() { echo "ERROR: $*" >&2; exit 1; }
info() { echo "[INFO] $*"; }

detect_os() {
    uname_out="$(uname -s)"
    case "${uname_out}" in
        Darwin) echo "macos" ;;
        Linux)
            if [ -f /etc/os-release ]; then
                . /etc/os-release
                echo "${ID:-linux}"
            else
                echo "linux"
            fi
            ;;
        *) echo "unknown" ;;
    esac
}

install_ubuntu() {
    info "Installing for Ubuntu (Qt6 + latest clang from LLVM apt repo)"
    if [ "$(id -u)" -ne 0 ]; then
        SUDO=sudo
    else
        SUDO=
    fi

    $SUDO apt-get update
    $SUDO apt-get install -y --no-install-recommends \
        ca-certificates curl gnupg lsb-release software-properties-common

    # Use the official apt.llvm.org install script to set up LLVM packages.
    # This follows the recommended installation method from apt.llvm.org.
    info "Installing llvm via official apt.llvm.org script..."
    # Ensure wget is available for the install script (script uses wget in its recommended usage)
    $SUDO apt-get update
    $SUDO apt-get install -y --no-install-recommends wget

    # Run the official installer script. Run as root when necessary via sudo.
    $SUDO bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"

    info "Installing Qt 6 packages (apt)..."
    # Qt6 package names vary by Ubuntu version; attempt to install common Qt6 dev packages.
    # Install including recommended packages because qmake/qttools can be pulled in as recommends.
    if ! $SUDO apt-get install -y \
        qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools qt6-qmake; then
        info "Some qt6 packages failed to install via apt. They may not be available on this Ubuntu release."
        info "Possible options: add a Qt PPA, use the official Qt online installer, or install a packaged Qt provided by your distro."
    else
        info "Qt packages installed (apt)."
    fi

    # Verify whether qmake or qtpaths are available; if not, give the user clear next steps.
    if command -v qmake >/dev/null 2>&1; then
        info "Found qmake: $(command -v qmake)"
    elif command -v qtpaths >/dev/null 2>&1; then
        info "Found qtpaths: $(command -v qtpaths)"
    else
        info "qmake and qtpaths not found. Qt may not be installed or may be installed in a non-standard location."
        echo "Suggested next steps:"
        echo " - Try installing a Qt PPA (example):"
        echo "     sudo add-apt-repository ppa:beineri/opt-qt-6 && sudo apt-get update && sudo apt-get install qt6-base-dev qt6-qmake"
        echo " - Or use the official Qt online installer (interactive): https://www.qt.io/download"
        echo " - If you installed Qt manually, add its bin folder to PATH, e.g.: export PATH=\"/opt/qt/<version>/bin:\$PATH\""
    fi

    info "Ubuntu install finished. Verify with: clang --version && qmake --version || qtpaths --version"
}

install_macos() {
    info "Installing for macOS (Homebrew) - Qt 6 and latest clang (llvm)"
    if ! command -v brew >/dev/null 2>&1; then
        info "Homebrew not found. Installing Homebrew (non-interactive)..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        # Add brew to PATH for this session (handle Intel vs Apple Silicon)
        if [ -d /opt/homebrew/bin ]; then
            eval "$(/opt/homebrew/bin/brew shellenv)"
        elif [ -d /usr/local/bin ]; then
            eval "$(/usr/local/bin/brew shellenv 2>/dev/null || true)"
        fi
    fi

    brew update

    info "Installing llvm (Homebrew 'llvm' provides clang)..."
    brew install llvm

    info "Installing Qt 6 via Homebrew..."
    # Homebrew keeps Qt 6 in 'qt' or sometimes 'qt@6' — try both
    if brew info qt >/dev/null 2>&1; then
        brew install qt
    else
        brew install qt@6
    fi

    llvm_prefix="$(brew --prefix llvm 2>/dev/null || true)"
    qt_prefix="$(brew --prefix qt 2>/dev/null || brew --prefix qt@6 2>/dev/null || true)"

    echo
    info "Post-install notes:"
    if [ -n "$llvm_prefix" ]; then
        echo " - Add llvm to PATH to use Homebrew clang:"
        echo "   export PATH=\"$llvm_prefix/bin:\$PATH\""
        echo "   (or add the above to your shell profile)"
    fi
    if [ -n "$qt_prefix" ]; then
        echo " - Qt binaries are in: $qt_prefix/bin"
        echo "   You may want to add to your PATH or use Qt tools by full path."
        echo "   Example: export PATH=\"$qt_prefix/bin:\$PATH\""
    fi

    info "macOS install finished. Verify with: clang --version && qmake --version || qtpaths --version"
}

main() {
    requested="${OS_ARG}"
    if [ -z "$requested" ]; then
        detected="$(detect_os)"
        case "$detected" in
            ubuntu|debian|linux)
                requested="ubuntu"
                ;;
            macos|darwin)
                requested="macos"
                ;;
            *)
                echo "Usage: $SCRIPT_NAME [ubuntu|macos]"
                echo "Detected OS: $detected"
                exit 1
                ;;
        esac
    fi

    case "$requested" in
        ubuntu)
            install_ubuntu
            ;;
        macos)
            install_macos
            ;;
        # all)
        #     install_ubuntu
        #     install_macos
        #     ;;
        *)
            echo "Unknown option: $requested"
            echo "Usage: $SCRIPT_NAME [ubuntu|macos|all]"
            exit 1
            ;;
    esac
}

main