#!/usr/bin/env bash
# bootstrap-hydra.sh

#   Script to setup a Distrobox called "hydra-dev" to build and run Hydra executables
#   Based on Fedora 43 image

set -euo pipefail

# --- Config -----------------------------------------------------
BOX_NAME="${BOX_NAME:-hydra-dev}"
IMAGE="${IMAGE:-registry.fedoraproject.org/fedora-toolbox:43}"
WORKDIR_HOST="${WORKDIR_HOST:-$PWD}"
WORKDIR_IN_BOX="${WORKDIR_IN_BOX:-$PWD}"

REPO_URL="${REPO_URL:-https://github.com/MultithreadCorner/Hydra.git}"
REPO_DIR_NAME="${REPO_DIR_NAME:-Hydra}"

CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
BUILD_TESTING="${BUILD_TESTING:-ON}"
DO_INSTALL="${DO_INSTALL:-false}"

# --- CUDA settings ----------------------------------------------
CUDA_ENABLE="${CUDA_ENABLE:-1}" # 1 enables CUDA
HOST_TOOLKIT="${HOST_TOOLKIT:-1}" # 1 mounts toolkit from the host, 0 installs it in the box
CUDA_HOST_PREFIX="${CUDA_HOST_PREFIX:-/usr/local/cuda}"  # toolkit path in the host

# exposing toolkit paths:
CUDA_MOUNTS=(
  "$CUDA_HOST_PREFIX:$CUDA_HOST_PREFIX:rw"
  "/usr/lib64/nvidia:/usr/lib64/nvidia:ro"
)

# NVIDIA devices to expose
NVIDIA_DEVICES=(
  "/dev/nvidiactl"
  "/dev/nvidia-uvm"
)
for d in /dev/nvidia[0-9]*; do
  [[ -e "$d" ]] && NVIDIA_DEVICES+=("$d")
done

# CUDA packages if HOST_TOOLKIT=0
DNF_PKGS_CUDA=(
  cuda-toolkit
)

# --- CPU settings -----------------------------------------------
# the user should add manually root and root-minuit2 if needed
DNF_PKGS=(
  git cmake gcc-c++ gcc make pkgconf-pkg-config which
  binutils tbb-devel fftw-devel tclap
  gsl-devel libomp-devel
)

# --- Helpers ----------------------------------------------------
need_cmd(){ command -v "$1" >/dev/null 2>&1 || { echo "Error:'$1' is needed in the host."; exit 1; }; }
dbox_exists(){ distrobox list --no-color 2>/dev/null | awk '{print $1}' | grep -qx "$BOX_NAME"; }
dbox_enter(){ distrobox enter --name "$BOX_NAME" -- "$@"; }

# --- Pre-check --------------------------------------------------
need_cmd distrobox
if [[ "$CUDA_ENABLE" == "1" ]]; then
  if ! command -v nvidia-smi >/dev/null 2>&1; then
    echo "Warning: 'nvidia-smi' not found. Disabling CUDA support."
    CUDA_ENABLE=0
  fi
fi

echo "==> Bootstrap Hydra (CUDA_ENABLE=$CUDA_ENABLE, HOST_TOOLKIT=$HOST_TOOLKIT) in box '$BOX_NAME'"

# --- Compose additional flags for Distrobox ---------------------
ADD_FLAGS=()

# Hook NVIDIA - inject device and libraries
if [[ "$CUDA_ENABLE" == "1" ]]; then
  ADD_FLAGS+=(--env NVIDIA_VISIBLE_DEVICES=all)
  ADD_FLAGS+=(--env NVIDIA_DRIVER_CAPABILITIES=all)
  [[ -d /usr/share/containers/oci/hooks.d ]] && ADD_FLAGS+=(--hooks-dir=/usr/share/containers/oci/hooks.d)

  # Fallback: manually exposing devices
  for dev in "${NVIDIA_DEVICES[@]}"; do
    [[ -e "$dev" ]] && ADD_FLAGS+=(--device "$dev")
  done

  # Mount toolkit from the host
  if [[ "$HOST_TOOLKIT" == "1" ]]; then
    if [[ -d "$CUDA_HOST_PREFIX" ]]; then
      for mnt in "${CUDA_MOUNTS[@]}"; do
        ADD_FLAGS+=(--volume "$mnt")
      done
      # Propagate PATH/LD_LIBRARY_PATH in the box
      ADD_FLAGS+=(--env PATH="$CUDA_HOST_PREFIX/bin:\$PATH")
      ADD_FLAGS+=(--env LD_LIBRARY_PATH="$CUDA_HOST_PREFIX/lib64:$CUDA_HOST_PREFIX/lib:\$LD_LIBRARY_PATH")
    else
      echo "Warning: CUDA_HOST_PREFIX='$CUDA_HOST_PREFIX' not found in the host. I'll try to install it in the box."
      HOST_TOOLKIT=0
    fi
  fi
fi

# --- Create Distrobox ------------------------------------------
if dbox_exists; then
  echo "==> Distrobox '$BOX_NAME' already exists."
else
  echo "==> Creating Distrobox '$BOX_NAME'..."
  distrobox create --name "$BOX_NAME" --image "$IMAGE" \
    --volume "$WORKDIR_HOST:$WORKDIR_IN_BOX:rw" \
    "${ADD_FLAGS[@]}"
  echo "==> Distrobox created."
fi

# --- Install deps inside box -----------------------------------
echo "==> Installing dependecies..."
dbox_enter sudo dnf -y update
dbox_enter sudo dnf -y install "${DNF_PKGS[@]}" || true

if [[ "$CUDA_ENABLE" == "1" && "$HOST_TOOLKIT" == "0" ]]; then
  echo "==> Trying to install CUDA toolkit within the box..."
  dbox_enter sudo dnf -y install "${DNF_PKGS_CUDA[@]}" || {
    echo "Warning: 'cuda-toolkit' not available in the current box repository"
    echo " - Option A: enable repo NVIDIA in the box and run again the script."
    echo " - Option B: use HOST_TOOLKIT=1 to mount the toolkit from the host"
  }
fi

# --- CUDA sanity check -----------------------------------------
if [[ "$CUDA_ENABLE" == "1" ]]; then
  echo "==> Checking CUDA inside the box (nvidia-smi and nvcc)..."
  set +e
  dbox_enter bash --noprofile --norc "nvidia-smi || true"
  dbox_enter bash --noprofile --norc "command -v nvcc && nvcc --version || echo 'nvcc not found'" || true
  set -e
fi

# --- Configure & pre build Hydra --------------------------------
echo "==> Configuring Hydra (CMake: $CMAKE_BUILD_TYPE, TESTING=$BUILD_TESTING, CUDA=$CUDA_ENABLE)..."
CMAKE_FLAGS=(
  -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
  -DBUILD_TESTING="$BUILD_TESTING"
  -DBUILD_DOXYGEN_DOCUMENTATION=OFF
)
[[ "$CUDA_ENABLE" == "1" ]] && CMAKE_FLAGS+=(-DHYDRA_ENABLE_CUDA=ON)

dbox_enter bash --noprofile --norc -o pipefail -e -u -c "
  mkdir -p '$WORKDIR_IN_BOX/build' && \
  cd '$WORKDIR_IN_BOX/build' && \
  cmake ${CMAKE_FLAGS[*]} ../
"

echo
echo "READY!"
echo
echo "Enter the distrobox with:"
echo "    distrobox enter $BOX_NAME"
echo
echo "To build and run an example:"
echo "    cd $WORKDIR_IN_BOX/build"
echo "    make sample_distribution_tbb"
echo "    ./examples/random/sample_distribution_tbb -n=10000000"
echo
echo "Paths:"
echo "  - source: $WORKDIR_IN_BOX"
echo "  - build: $WORKDIR_IN_BOX/build"