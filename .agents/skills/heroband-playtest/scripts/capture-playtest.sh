#!/bin/sh
set -eu

usage() {
	cat <<'EOF'
Usage: capture-playtest.sh --state-dir DIR [options]

Options:
  --state-dir DIR     State directory containing playtest.env.
  --session NAME      Override tmux session from playtest.env.
  --lines N           Number of recent lines to print. Defaults to 80.
  -h, --help          Show this help.
EOF
}

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
		*)
			echo "Unknown argument: $1" >&2
			usage >&2
			exit 2
			;;
	esac
done

if [ -z "$state_dir" ]; then
	echo "Missing required --state-dir DIR." >&2
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

mkdir -p "$state_dir"
stamp=$(date +%Y%m%d-%H%M%S)
capture_file="$state_dir/capture-$stamp.txt"

tmux capture-pane -p -t "$session" > "$capture_file"
{
	echo "===== capture $stamp session=$session ====="
	cat "$capture_file"
	echo
} >> "$state_dir/transcript.txt"

tail -n "$lines" "$capture_file"
