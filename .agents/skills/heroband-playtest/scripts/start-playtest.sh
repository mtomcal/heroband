#!/bin/sh
set -eu

usage() {
	cat <<'EOF'
Usage: start-playtest.sh --contract FILE [options]

Options:
  --contract FILE     Required test contract written before launch.
  --state-dir DIR     State/artifact directory. Defaults to mktemp.
  --build-dir DIR     GCU build directory. Defaults to <repo>/build-gcu-test.
  --session NAME      tmux session name. Defaults to heroband-playtest-<timestamp>.
  --no-build          Do not configure or build; require existing executable.
  -h, --help          Show this help.
EOF
}

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)

contract=
state_dir=
build_dir="$repo_root/build-gcu-test"
session="heroband-playtest-$(date +%Y%m%d-%H%M%S)"
do_build=1

while [ "$#" -gt 0 ]; do
	case "$1" in
		--contract)
			contract=${2:?"--contract requires a file"}
			shift 2
			;;
		--state-dir)
			state_dir=${2:?"--state-dir requires a directory"}
			shift 2
			;;
		--build-dir)
			build_dir=${2:?"--build-dir requires a directory"}
			shift 2
			;;
		--session)
			session=${2:?"--session requires a name"}
			shift 2
			;;
		--no-build)
			do_build=0
			shift
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			echo "Unknown argument: $1" >&2
			usage >&2
			exit 2
			;;
	esac
done

if [ -z "$contract" ]; then
	echo "Missing required --contract FILE. Write the test contract before launching tmux." >&2
	exit 2
fi

if [ ! -s "$contract" ]; then
	echo "Contract file is missing or empty: $contract" >&2
	exit 2
fi

if ! command -v tmux >/dev/null 2>&1; then
	echo "tmux is required for direct gameplay testing." >&2
	exit 1
fi

if [ -z "$state_dir" ]; then
	state_dir=$(mktemp -d /tmp/heroband-playtest.XXXXXX)
else
	mkdir -p "$state_dir"
fi

contract_abs=$(CDPATH= cd -- "$(dirname -- "$contract")" && pwd)/$(basename -- "$contract")
state_contract="$state_dir/TEST_CONTRACT.md"
if [ "$contract_abs" != "$state_contract" ]; then
	cp "$contract_abs" "$state_contract"
fi

if [ "$do_build" -eq 1 ]; then
	if [ ! -f "$build_dir/CMakeCache.txt" ]; then
		if command -v ninja >/dev/null 2>&1; then
			cmake -G Ninja -B "$build_dir" -DSUPPORT_GCU_FRONTEND=ON -DSUPPORT_TEST_FRONTEND=ON "$repo_root"
		else
			cmake -B "$build_dir" -DSUPPORT_GCU_FRONTEND=ON -DSUPPORT_TEST_FRONTEND=ON "$repo_root"
		fi
	else
		if ! grep -q '^SUPPORT_GCU_FRONTEND:BOOL=ON$' "$build_dir/CMakeCache.txt"; then
			echo "$build_dir exists but SUPPORT_GCU_FRONTEND is not ON." >&2
			echo "Use a dedicated GCU build directory or reconfigure explicitly." >&2
			exit 1
		fi
	fi
	cmake --build "$build_dir" -j2
fi

exe="$build_dir/game/angband"
if [ ! -x "$exe" ]; then
	echo "Expected executable is missing: $exe" >&2
	exit 1
fi

if tmux has-session -t "$session" 2>/dev/null; then
	echo "tmux session already exists: $session" >&2
	exit 1
fi

cat > "$state_dir/playtest.env" <<EOF
SESSION=$session
STATE_DIR=$state_dir
BUILD_DIR=$build_dir
EXECUTABLE=$exe
REPO_ROOT=$repo_root
CONTRACT=$state_dir/TEST_CONTRACT.md
EOF

tmux new-session -d -s "$session" -c "$repo_root" \
	"HOME='$state_dir' TERM=xterm-256color '$exe' -mgcu"

{
	echo "Started tmux session: $session"
	echo "State directory: $state_dir"
	echo "Executable: $exe"
	echo "Contract: $state_dir/TEST_CONTRACT.md"
	echo "Capture with: $script_dir/capture-playtest.sh --state-dir '$state_dir'"
} | tee "$state_dir/start.log"
