#!/bin/sh
set -eu

usage() {
	cat <<'EOF'
Usage: send-playtest-key.sh --state-dir DIR [options] KEY [KEY...]

Options:
  --state-dir DIR     State directory containing playtest.env.
  --session NAME      Override tmux session from playtest.env.
  --lines N           Number of recent captured lines to print. Defaults to 80.
  -h, --help          Show this help.

KEY values are passed to tmux send-keys, such as Space, Enter, C-c, Up, Down, a.
EOF
}

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

state_dir=
session=
lines=80

while [ "$#" -gt 0 ]; do
	case "$1" in
		--state-dir)
			state_dir=${2:?"--state-dir requires a directory"}
			shift 2
			;;
		--session)
			session=${2:?"--session requires a name"}
			shift 2
			;;
		--lines)
			lines=${2:?"--lines requires a count"}
			shift 2
			;;
		-h|--help)
			usage
			exit 0
			;;
		--)
			shift
			break
			;;
		-*)
			echo "Unknown argument: $1" >&2
			usage >&2
			exit 2
			;;
		*)
			break
			;;
	esac
done

if [ -z "$state_dir" ]; then
	echo "Missing required --state-dir DIR." >&2
	exit 2
fi

if [ "$#" -lt 1 ]; then
	echo "Missing KEY argument." >&2
	exit 2
fi

if [ -z "$session" ]; then
	if [ ! -f "$state_dir/playtest.env" ]; then
		echo "Missing playtest.env in state directory: $state_dir" >&2
		exit 2
	fi
	session=$(sed -n 's/^SESSION=//p' "$state_dir/playtest.env")
fi

if [ -z "$session" ]; then
	echo "Could not determine tmux session." >&2
	exit 2
fi

if ! tmux has-session -t "$session" 2>/dev/null; then
	echo "tmux session is not running: $session" >&2
	exit 1
fi

tmux send-keys -t "$session" "$@"
"$script_dir/capture-playtest.sh" --state-dir "$state_dir" --session "$session" --lines "$lines"
