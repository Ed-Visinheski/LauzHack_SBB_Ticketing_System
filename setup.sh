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

    # Detect latest LLVM major version via GitHub releases (best-effort)
    info "Detecting latest LLVM release..."
    latest_tag="$(curl -fsSL "https://api.github.com/repos/llvm/llvm-project/releases/latest" 2>/dev/null | grep -Eo '"tag_name":\s*"[^"]+"' | sed -E 's/.*"([^"]+)".*/\1/' || true)"
    if [ -z "$latest_tag" ]; then
        info "Could not detect latest LLVM release from GitHub, falling back to 'clang' package."
        LLVM_MAJOR=""
    else
        # examples of tag: llvmorg-17.0.0 or 17.0.0 ; extract first number group as major
        major="$(echo "$latest_tag" | grep -oE '[0-9]+' | head -n1 || true)"
        if [ -n "$major" ]; then
            LLVM_MAJOR="$major"
            info "Detected LLVM major version: $LLVM_MAJOR (from tag $latest_tag)"
        else
            LLVM_MAJOR=""
        fi
    fi

    if [ -n "$LLVM_MAJOR" ]; then
        info "Adding LLVM apt repository for clang-$LLVM_MAJOR..."
        # Add apt key and repo (apt-key used for simplicity; modern systems may warn)
        curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | $SUDO apt-key add -
        distro="$(lsb_release -sc)"
        echo "deb http://apt.llvm.org/${distro}/ llvm-toolchain-${LLVM_MAJOR} main" | $SUDO tee /etc/apt/sources.list.d/llvm.list >/dev/null
        $SUDO apt-get update
        info "Installing clang-$LLVM_MAJOR and related tools..."
        $SUDO apt-get install -y --no-install-recommends clang-"${LLVM_MAJOR}" lldb-"${LLVM_MAJOR}" clang-format-"${LLVM_MAJOR}"
        # Prefer clang-$LLVM_MAJOR as default via update-alternatives
        if [ -x "/usr/bin/clang-${LLVM_MAJOR}" ]; then
            $SUDO update-alternatives --install /usr/bin/clang clang /usr/bin/clang-"${LLVM_MAJOR}" 100
        fi
        if [ -x "/usr/bin/clang++-${LLVM_MAJOR}" ]; then
            $SUDO update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-"${LLVM_MAJOR}" 100
        fi
    else
        info "Installing distro 'clang' package (may not be the very latest)..."
        $SUDO apt-get update
        $SUDO apt-get install -y --no-install-recommends clang
    fi

    info "Installing Qt 6 packages (apt)..."
    # Qt6 package names vary by Ubuntu version; install common Qt6 dev packages
    $SUDO apt-get install -y --no-install-recommends \
        qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools qt6-qmake || {
            info "Some qt6 packages failed to install via apt. You may need to add a PPA or install Qt via the official installer."
        }

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