#!/bin/sh
set -eu

usage() {
	cat <<'EOF'
Usage: stop-playtest.sh --state-dir DIR [options]

Options:
  --state-dir DIR     State directory containing playtest.env.
  --session NAME      Override tmux session from playtest.env.
  --keep              Leave the tmux session running.
  -h, --help          Show this help.
EOF
}

state_dir=
session=
keep=0

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
		--keep)
			keep=1
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

if [ "$keep" -eq 1 ]; then
	echo "Leaving tmux session running: $session"
	echo "State directory: $state_dir"
	exit 0
fi

if tmux has-session -t "$session" 2>/dev/null; then
	tmux capture-pane -p -t "$session" > "$state_dir/final-capture.txt" || true
	tmux kill-session -t "$session"
	echo "Stopped tmux session: $session"
else
	echo "tmux session was not running: $session"
fi

echo "Artifacts preserved in: $state_dir"
